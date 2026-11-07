#include <algorithm>
#include <cctype>
#include <charconv>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cursiv/syntax-tree/DocumentReferences.hpp>
#include <cursiv/utils/UriUtils.hpp>
#include <limits>
#include <map>
#include <memory>
#include <pugixml.hpp>
#include <set>
#include <sstream>
#include <stdexcept>
#include <xsmp/CopyrightNoticeProvider.hpp>
#include <xsmp/DeploymentSemantics.hpp>
#include <xsmp/SharedCoreServices.hpp>
#include <xsmp/Traversal.hpp>
#include <xsmp/ast.hpp>
#include <xsmp/references/QualifiedName.hpp>
#include <xsmp/smp/SmpService.hpp>
#include <xsmp/smp/SmpXmlText.hpp>
#include <xsmp/solver/Solver.hpp>
#include <xsmp/utils/FileWriter.hpp>
#include <xsmp/utils/XsmpDocumentation.hpp>
#include <xsmp/utils/XsmpUtils.hpp>
#include <xsmp/workspace/SourceUri.hpp>

namespace xsmp {

namespace {

using cursiv::AstNode;

template <class T>
const T* as(const AstNode* n) {
  return cursiv::ast_cast<T>(n);
}

void attribute(pugi::xml_node node, const char* key, std::string_view value) {
  const auto text = encodeXmlText(value);
  node.append_attribute(key).set_value(text.data(), text.size());
}

void element(pugi::xml_node node, const char* key, std::string_view value) {
  node.append_child(key).text().set(encodeXmlText(value).c_str());
}

// The canonical `xs:duration` of nanoseconds, as the other XSMP tools write
// it: `PT0S`, `P1D`, `-P2DT1M0.5S`. Only the whole duration has a sign.
std::string durationText(std::int64_t ns) {
  if (ns == 0) return "PT0S";
  constexpr std::uint64_t second = 1'000'000'000;
  constexpr std::uint64_t day = 86'400 * second;
  auto rest = ns < 0 ? std::uint64_t{0} - static_cast<std::uint64_t>(ns)
                     : static_cast<std::uint64_t>(ns);
  std::string result = ns < 0 ? "-P" : "P";
  if (const auto days = rest / day) {
    result += std::to_string(days) + "D";
    rest %= day;
    if (rest == 0) return result;
  }
  result += "T";
  if (const auto hours = rest / (3600 * second))
    result += std::to_string(hours) + "H";
  if (const auto minutes = rest / (60 * second) % 60)
    result += std::to_string(minutes) + "M";
  rest %= 60 * second;
  if (rest == 0) return result;
  result += std::to_string(rest / second);
  if (const auto fraction = rest % second) {
    auto digits = std::to_string(second + fraction).substr(1);
    while (digits.back() == '0') digits.pop_back();
    result += "." + digits;
  }
  return result + "S";
}

// `YYYY-MM-DDThh:mm:ss.fffZ` for `sinceEpoch` seconds after the Unix epoch,
// with `fraction` in `digits` fractional digits (3 or 9). A negative year is
// written
// `-YYYY`, as `xs:dateTime` requires.
std::string isoDateTime(std::chrono::seconds sinceEpoch, long long fraction,
                        int digits) {
  using namespace std::chrono;
  const sys_seconds instant{sinceEpoch};
  const auto day = floor<days>(instant);
  const year_month_day date{day};
  const hh_mm_ss time{instant - day};
  const auto year = int(date.year());
  char buffer[64];
  std::snprintf(
      buffer, sizeof(buffer), "%s%04d-%02u-%02uT%02lld:%02lld:%02lld.%0*lldZ",
      year < 0 ? "-" : "", year < 0 ? -year : year, unsigned(date.month()),
      unsigned(date.day()), static_cast<long long>(time.hours().count()),
      static_cast<long long>(time.minutes().count()),
      static_cast<long long>(time.seconds().count()), digits, fraction);

  return buffer;
}

// A DateTime value counts from the SMP reference time. Whole seconds first: the
// shift to the Unix epoch can leave 64 bits of nanoseconds.
std::string dateTimeText(std::int64_t ns) {
  using namespace std::chrono;
  const nanoseconds value{ns};
  const auto whole = floor<seconds>(value);
  return isoDateTime(whole + seconds{kDateTimeEpochSeconds},
                     (value - whole).count(), 9);
}

// The `Date` of a document, in milliseconds. Empty when `text` is not a date.
std::string dateMetadataText(std::string_view text) {
  using namespace std::chrono;
  const auto parsed = parseDateTimeMilliseconds(text);
  if (!parsed) return {};
  const milliseconds value{*parsed};
  const auto whole = floor<seconds>(value);
  return isoDateTime(whole, (value - whole).count(), 3);
}

// The shortest text that reads back as the same value, as `xs:double` spells
// it.
std::string real(double value) {
  if (std::isnan(value)) return "NaN";
  if (std::isinf(value)) return value < 0 ? "-INF" : "INF";
  char buffer[128];
  const auto result =
      std::to_chars(std::begin(buffer), std::end(buffer), value);
  if (result.ec != std::errc{})
    throw std::runtime_error("Cannot render SMP floating-point value");
  return {buffer, result.ptr};
}

// The copyright notice of the document, as an XML comment.
void copyrightNotice(pugi::xml_document& xml,
                     const cursiv::Document& document) {
  if (!document.language || !document.language->commentProvider
      || !document.parseResult.value)
    return;
  auto notice = getCopyrightNotice(*document.language->commentProvider,
                                   *document.parseResult.value);
  if (!notice) return;
  // An XML comment must not contain `--` nor end with `-`.
  for (std::size_t at = 0; at < notice->size();) {
    if ((*notice)[at] != '-') {
      ++at;
      continue;
    }
    auto end = notice->find_first_not_of('-', at);
    if (end == notice->npos) end = notice->size();
    if (end - at >= 2) notice->replace(at, end - at, end - at, '*');
    at = end;
  }
  if (notice->ends_with('-')) *notice += ' ';
  xml.append_child(pugi::node_comment).set_value(notice->c_str());
}

// The text of a path, without its `unsafe` keyword.
std::string path(const Path* value) {
  if (!value) return {};
  const auto segment = [](const PathSegment* s) -> std::string {
    if (auto* n = as<ConcretePathNamedSegment>(s))
      return std::string(n->reference.refText);
    if (auto* n = as<PatternPathNamedSegment>(s)) return std::string(n->text);
    if (as<PathParentSegment>(s)) return "..";
    if (as<PathSelfSegment>(s)) return ".";
    return {};
  };
  std::string result = value->absolute ? "/" : "";
  result += segment(value->head);
  for (auto* e : value->elements) {
    if (auto* n = as<PathMember>(e))
      result += std::string(n->separator) + segment(n->segment);
    else if (auto* n = as<PathIndex>(e))
      result += "[" + std::to_string(n->index) + "]";
  }
  return result;
}

// The name of `node`, empty when it has none.
std::string nodeName(const AstNode& node) {
  if (const auto* parameter = as<ReturnParameter>(&node))
    return std::string(parameter->name);
  return node.type && node.type->readName
             ? std::string(node.type->readName(node))
             : "";
}

const Document* rootOf(const AstNode* node) {
  for (; node; node = node->parent)
    if (auto* root = as<Document>(node)) return root;
  return nullptr;
}

// The name of the XML file of a document: `.xsmpcat` gives `.smpcat`, and so
// on.
std::string xmlFileName(const cursiv::Document& document) {
  std::filesystem::path file(cursiv::uri_file_name(document.uri));
  auto extension = file.extension().string();
  if (extension.starts_with(".xsmp")) extension.erase(1, 1);
  file.replace_extension(extension);
  return file.filename().string();
}

// A number as text.
std::string numberText(const SolverValue& value) {
  return isFloatingType(value.type) ? real(value.floating)
         : isSignedIntegral(value.type)
             ? std::to_string(static_cast<std::int64_t>(value.integral))
             : std::to_string(value.integral);
}

// `.` as the `::` of a C++ name.
std::string cppName(std::string name) {
  for (std::size_t at = 0; (at = name.find('.', at)) != name.npos; at += 2)
    name.replace(at, 1, "::");
  return name;
}

// Appends the XML of one document. `standard` decides the few names that differ
// between the revisions of the standard.
struct Writer {
  const cursiv::Document& document;
  std::string standard;

  Writer(const cursiv::Document& doc, std::string_view revision)
      : document(doc), standard(revision) {}

  std::string tag(const AstNode& node, std::string_view name) const {
    if (auto value = docTag(node, name)) return *value;
    return {};
  }

  bool hasTag(const AstNode& node, std::string_view name) const {
    return docTag(node, name).has_value();
  }

  // The XML `Id` of a node: its `@id` tag, else its qualified name. An
  // Operation adds the type of each parameter, which tells overloads apart, and
  // a Parameter adds its name to the `Id` of its Operation. A Catalogue starts
  // with `_`.
  std::string id(const AstNode& node) const {
    auto explicitId = tag(node, "id");
    if (!explicitId.empty()) return explicitId;
    auto result = qualifiedName(node);
    if (const auto* op = as<Operation>(&node); op && !op->parameter.empty()) {
      for (const auto* parameter : op->parameter)
        result +=
            "-"
            + (parameter->type.get() ? std::string(parameter->type.get()->name)
                                     : std::string{});
    }
    if ((as<Parameter>(&node) || as<ReturnParameter>(&node)) && node.parent) {
      auto name = nodeName(node);
      if (name.empty()) name = "return";
      return id(*node.parent) + "." + name;
    }
    if (result.empty()) result = nodeName(node);
    return as<Catalogue>(&node) ? "_" + result : result;
  }

  // The doc comment of `node`, as a `Description`.
  void describe(pugi::xml_node xml, const AstNode& node) const {
    if (const auto text = docDescription(node); text && !text->empty())
      element(xml, "Description", *text);
  }

  // What any named element has: `Id`, `Name`, `Description` and `Metadata`.
  void named(pugi::xml_node xml, const AstNode& node) const {
    attribute(xml, "Id", id(node));
    auto name = nodeName(node);
    if (name.empty() && as<ReturnParameter>(&node)) name = "return";
    attribute(xml, "Name", name);
    describe(xml, node);

    const auto* attributes =
        as<NamedElement>(&node)      ? &as<NamedElement>(&node)->attributes
        : as<ReturnParameter>(&node) ? &as<ReturnParameter>(&node)->attributes
                                     : nullptr;

    if (attributes)
      for (std::size_t i = 0; i < attributes->size(); ++i) {
        auto* a = (*attributes)[i];
        if (!a) continue;
        auto x = xml.append_child("Metadata");
        attribute(x, "xsi:type", "Types:Attribute");
        const auto* type = as<AttributeType>(a->type.get());
        const auto name = type ? std::string(type->name) : std::string{};
        attribute(x, "Name", name);
        attribute(x, "Id", id(node) + "." + name + "." + std::to_string(i));
        ref(x, "Type", a->type);
        // Without a value of its own, an attribute writes the default of its
        // type.
        const auto* value = a->value ? a->value
                            : type   ? type->default_
                                     : nullptr;
        if (!value)
          throw std::runtime_error("The attribute '" + name + "' of '"
                                   + id(node)
                                   + "' has no value to write in SMP XML");
        valueChild(x, "Value", value, type ? type->type.get() : nullptr);
      }
  }

  // The root element of a document: `named`, plus what its doc tags give.
  void metadata(pugi::xml_node xml, const AstNode& node) const {
    named(xml, node);
    if (const auto title = docTag(node, "title"))
      attribute(xml, "Title", *title);
    if (const auto date = docTag(node, "date")) {
      const auto text = dateMetadataText(*date);
      if (!text.empty()) attribute(xml, "Date", text);
    }
    // The creators are comma-separated.
    std::string creators;
    for (const auto& creator : docTagRanges(node, "creator"))
      creators += (creators.empty() ? "" : ", ") + creator.content;
    if (!creators.empty()) attribute(xml, "Creator", creators);
    if (const auto version = docTag(node, "version"))
      attribute(xml, "Version", *version);
  }

  // An XLink to `target`, or to `spelling` when it is unresolved. The `href` is
  // `#id` in the same document and `file#id` in another, with the XML extension
  // of that file or, for a builtin library, the name that the standard gives
  // it.
  void xlink(pugi::xml_node xml, const AstNode* target,
             std::string_view spelling) const {
    const auto name = target ? nodeName(*target) : std::string(spelling);
    attribute(xml, "xlink:title", name);
    auto href = target ? id(*target) : std::string(spelling);
    const auto* targetRoot = rootOf(target);
    if (targetRoot && targetRoot->getDocument() != &document) {
      const auto& uri = targetRoot->getDocument()->uri;
      const auto filename = cursiv::uri_file_name(uri);
      // The 2025 level-1 library keeps the namespace of the 2019 standard.
      if (isBuiltinUri(uri)
          && (filename == "ecss.smp@ECSS_SMP_2020.xsmpcat"
              || filename == "ecss.smp.l1@ECSS_SMP_2025.xsmpcat"))
        href = "http://www.ecss.nl/smp/2019/Smdl#" + href;
      else if (isBuiltinUri(uri)
               && filename == "ecss.smp.l2@ECSS_SMP_2025.xsmpcat")
        href = "ecss.smp.l2.smpcat#" + href;
      else
        href = xmlFileName(*targetRoot->getDocument()) + "#" + href;
    } else
      href = "#" + href;
    attribute(xml, "xlink:href", href);
  }

  template <class T>
  void ref(pugi::xml_node xml, const char* key,
           const cursiv::reference<T>& value) const {
    xlink(xml.append_child(key), value.get(), value.refText);
  }

  // A value of `xsi:type` `Types:<type>`, in a `Value` attribute.
  void scalar(pugi::xml_node xml, std::string_view type,
              std::string_view value) const {
    attribute(xml, "xsi:type", "Types:" + std::string(type));
    attribute(xml, "Value", value);
  }

  // `expr` as the child `key` of `xml`, left out when it has no value or is
  // `nullptr`, the one value that XML has no form for: `Types:Value` is
  // abstract. A value left out means the same.
  void valueChild(pugi::xml_node xml, const char* key, const Expression* expr,
                  const Type* expected) const {
    if (expr && !isNullptr(expr))
      expression(xml.append_child(key), expr, expected);
  }

  // A catalogue expression as a typed value of the `expected` type.
  void expression(pugi::xml_node xml, const Expression* expr,
                  const Type* expected) const {
    if (auto* designated = as<DesignatedInitializer>(expr)) {
      attribute(xml, "Field", designated->field.refText);
      auto* field = designated->field.get();
      expression(xml, designated->expr, field ? field->type.get() : expected);
      return;
    }

    if (auto* collection = as<CollectionLiteral>(expr)) {
      const auto* array = as<Array>(expected);
      const auto* structure = as<Structure>(expected);
      const auto itemKind = array ? getPTK(array->itemType.get()) : PTK::None;
      const auto simple = array && itemKind != PTK::None;
      attribute(
          xml, "xsi:type",
          simple  ? "Types:"
                        + std::string(itemKind == PTK::Enum ? "Enumeration"
                                                            : ptkName(itemKind))
                        + "ArrayValue"
          : array ? "Types:ArrayValue"
                  : "Types:StructureValue");
      std::vector<const Field*> fields;
      if (structure) fields = getAllFields(*structure);
      for (std::size_t i = 0; i < collection->elements.size(); ++i) {
        const auto* entry = collection->elements[i];
        const auto* designated = as<DesignatedInitializer>(entry);
        if (isNullptr(designated ? designated->expr : entry)) continue;
        auto item = xml.append_child(array ? "ItemValue" : "FieldValue");
        const auto* type = array               ? array->itemType.get()
                           : i < fields.size() ? fields[i]->type.get()
                                               : nullptr;
        // A value in a structure names its field, even when written by
        // position.
        if (!array && !designated && i < fields.size())
          attribute(item, "Field", fields[i]->name);
        expression(item, entry, type);
        // In a simple array the items are bare values: the array names their
        // type.
        if (simple) {
          item.remove_attribute("xsi:type");
          // A simple array of enumeration values carries only the numbers.
          if (itemKind == PTK::Enum) item.remove_attribute("Literal");
        }
      }
      return;
    }

    auto value = expected ? getValueAs(expr, *expected) : getValue(expr);
    if (!value)
      throw std::runtime_error(
          "Cannot evaluate catalogue value for SMP export at '"
          + (expr && expr->parent ? id(*expr->parent) : std::string{})
          + "' (expected '"
          + (expected ? qualifiedName(*expected) : std::string{}) + "')");

    auto kind = value->type;
    if (kind == PTK::Enum) {
      if (!value->literal)
        throw std::runtime_error(
            "Unresolved enumeration literal in SMP export");
      auto numeric = getValue(value->literal->value);
      scalar(xml, "EnumerationValue",
             numeric
                 ? std::to_string(static_cast<std::int64_t>(numeric->integral))
                 : "0");
      attribute(xml, "Literal", value->literal->name);
      return;
    }

    if (kind == PTK::Bool)
      scalar(xml, "BoolValue", value->boolean ? "true" : "false");
    else if (kind == PTK::Char8 || kind == PTK::String8)
      scalar(xml, std::string(ptkName(kind)) + "Value", value->text);
    else if (kind == PTK::Duration)
      scalar(xml, "DurationValue",
             durationText(static_cast<std::int64_t>(value->integral)));
    else if (kind == PTK::DateTime)
      scalar(xml, "DateTimeValue",
             dateTimeText(static_cast<std::int64_t>(value->integral)));
    else if (isFloatingType(kind) || isIntegralType(kind))
      scalar(xml, std::string(ptkName(kind)) + "Value", numberText(*value));
    else
      throw std::runtime_error("Unsupported catalogue value in SMP export");
  }

  // A catalogue expression as the text of a C++ expression, which is what a
  // Parameter has as its `Default` in ECSS SMP 2025.
  std::string cppExpression(const Expression* expr,
                            const Type* expected) const {
    if (isNullptr(expr)) return "nullptr";
    if (const auto* designated = as<DesignatedInitializer>(expr)) {
      const auto* field = designated->field.get();
      return "." + std::string(designated->field.refText) + " = "
             + cppExpression(designated->expr,
                             field ? field->type.get() : expected);
    }

    if (const auto* collection = as<CollectionLiteral>(expr)) {
      const auto* array = as<Array>(expected);
      std::vector<const Field*> fields;
      if (const auto* structure = as<Structure>(expected))
        fields = getAllFields(*structure);
      std::string result = "{";
      for (std::size_t i = 0; i < collection->elements.size(); ++i)
        result += (i ? ", " : " ")
                  + cppExpression(collection->elements[i],
                                  array               ? array->itemType.get()
                                  : i < fields.size() ? fields[i]->type.get()
                                                      : nullptr);
      return result + " }";
    }

    const auto value = expected ? getValueAs(expr, *expected) : getValue(expr);
    if (!value)
      throw std::runtime_error("Cannot evaluate a parameter default at '"
                               + (expr->parent ? id(*expr->parent) : "") + "'");
    switch (value->type) {
      case PTK::Bool:
        return value->boolean ? "true" : "false";
      case PTK::Char8:
        return "'" + cppEscaped(value->text, '\'') + "'";
      case PTK::String8:
        return "\"" + cppEscaped(value->text) + "\"";
      case PTK::Enum:
        if (!value->literal)
          throw std::runtime_error(
              "Unresolved enumeration literal in SMP export");
        return cppName(qualifiedName(*value->literal));
      default:
        return numberText(*value);
    }
  }

  // Writes `v` as the scalar `kind` when it is a `T`, and tells whether it was.
  template <class T>
  bool writeScalar(pugi::xml_node xml, const Value* v, const char* kind) const {
    const auto* typed = as<T>(v);
    if (typed == nullptr) return false;
    if constexpr (std::is_arithmetic_v<decltype(typed->value)>)
      scalar(xml, kind, std::to_string(typed->value));
    else
      scalar(xml, kind, typed->value);
    return true;
  }

  // A deployment value. An `IntValue` or a `FloatValue` takes its kind from the
  // type that its place expects, and is a bare `Types:Value` when that type
  // does not fit.
  void value(pugi::xml_node xml, const Value* v) const {
    if (!v) throw std::runtime_error("Missing deployment value in SMP export");
    if (writeScalar<Int8Value>(xml, v, "Int8Value")
        || writeScalar<Int16Value>(xml, v, "Int16Value")
        || writeScalar<Int32Value>(xml, v, "Int32Value")
        || writeScalar<Int64Value>(xml, v, "Int64Value")
        || writeScalar<UInt8Value>(xml, v, "UInt8Value")
        || writeScalar<UInt16Value>(xml, v, "UInt16Value")
        || writeScalar<UInt32Value>(xml, v, "UInt32Value")
        || writeScalar<UInt64Value>(xml, v, "UInt64Value"))
      return;

    if (auto* n = as<IntValue>(v)) {
      const auto expected = getPTK(expectedValueType(n));
      if (isIntegralType(expected) && expected != PTK::DateTime
          && expected != PTK::Duration)
        scalar(xml, std::string(ptkName(expected)) + "Value", n->value);
      else
        attribute(xml, "xsi:type", "Types:Value");
      return;
    }

    if (auto* n = as<Float32Value>(v)) {
      scalar(xml, "Float32Value", real(n->value));
      return;
    }

    if (auto* n = as<Float64Value>(v)) {
      scalar(xml, "Float64Value", real(n->value));
      return;
    }

    if (auto* n = as<FloatValue>(v)) {
      const auto expected = getPTK(expectedValueType(n));
      if (expected == PTK::Float32 || expected == PTK::Float64)
        scalar(xml, expected == PTK::Float32 ? "Float32Value" : "Float64Value",
               real(n->value));
      else
        attribute(xml, "xsi:type", "Types:Value");
      return;
    }

    if (auto* n = as<BoolValue>(v)) {
      scalar(xml, "BoolValue", n->value ? "true" : "false");
      return;
    }
    if (writeScalar<Char8Value>(xml, v, "Char8Value")
        || writeScalar<String8Value>(xml, v, "String8Value")
        || writeScalar<DurationValue>(xml, v, "DurationValue")
        || writeScalar<DateTimeValue>(xml, v, "DateTimeValue"))
      return;

    if (auto* n = as<EnumerationValue>(v)) {
      if (n->value) {
        scalar(xml, "EnumerationValue", std::to_string(*n->value));
        return;
      }
      auto* literal = n->reference.get();
      const auto numeric =
          literal ? getValue(literal->value) : std::optional<SolverValue>{};
      attribute(xml, "xsi:type", "Types:EnumerationValue");
      if (numeric)
        attribute(xml, "Value",
                  std::to_string(static_cast<std::int64_t>(numeric->integral)));
      // The Literal attribute names the literal that the value of this
      // literal refers to; a plain number has none.
      if (numeric && numeric->type == PTK::Enum && numeric->literal)
        attribute(xml, "Literal", numeric->literal->name);
      return;
    }

    if (auto* n = as<FieldValue>(v)) {
      if (n->field) attribute(xml, "Field", path(n->field));
      value(xml, n->value);
      return;
    }

    if (auto* n = as<CfgStructureFieldValue>(v)) {
      if (n->field) attribute(xml, "Field", n->field->reference.refText);
      value(xml, n->value);
      return;
    }

    if (auto* n = as<ArrayValue>(v)) {
      const auto* array = as<Array>(expectedValueType(n));
      const auto itemKind = array ? getPTK(array->itemType.get()) : PTK::None;
      const auto simple =
          array && itemKind != PTK::None && isSimpleArrayAttribute(*array);
      attribute(
          xml, "xsi:type",
          simple ? "Types:"
                       + std::string(itemKind == PTK::Enum ? "Enumeration"
                                                           : ptkName(itemKind))
                       + "ArrayValue"
                 : "Types:ArrayValue");
      for (auto* item : n->elements) value(xml.append_child("ItemValue"), item);
      if (n->startIndex.value_or(0) == 0) return;

      // Only a simple array of a Configuration of ECSS SMP 2025 has a start
      // index. Without a known type, items that are all of one simple kind give
      // it: `Types:Int32Value` gives `Types:Int32ArrayValue`.
      std::string kind = xml.first_child().attribute("xsi:type").value();
      const auto typed =
          !array && kind.ends_with("Value") && kind != "Types:Value"
          && !kind.ends_with("ArrayValue") && kind != "Types:StructureValue"
          && std::ranges::all_of(xml.children("ItemValue"), [&](auto item) {
               return kind == item.attribute("xsi:type").value();
             });
      if (standard != "ECSS_SMP_2025" || !as<Configuration>(rootOf(n))
          || (!simple && !typed))
        throw std::runtime_error(
            "A start index cannot be written here: only a simple array of a "
            "Configuration of ECSS SMP 2025 has one");
      if (typed) {
        kind.insert(kind.size() - 5, "Array");
        xml.attribute("xsi:type").set_value(kind.c_str());
      }
      xml.prepend_child("StartIndex")
          .text()
          .set(std::to_string(*n->startIndex).c_str());
      return;
    }

    if (auto* n = as<StructureValue>(v)) {
      attribute(xml, "xsi:type", "Types:StructureValue");
      // A value in a structure names its field: one without a designator takes
      // the next field not named so far.
      const auto fields = valueFields(expectedValueType(n));
      std::set<std::string, std::less<>> used;
      std::size_t next = 0;
      for (auto* item : n->elements) {
        auto child = xml.append_child("FieldValue");
        std::string designator;
        if (const auto* field = as<CfgStructureFieldValue>(item);
            field && field->field)
          designator = field->field->reference.refText;
        else if (const auto* field = as<FieldValue>(item);
                 field && field->field)
          designator = path(field->field);
        if (!designator.empty())
          used.insert(std::move(designator));
        else {
          while (next < fields.size() && used.contains(fields[next]->name))
            ++next;
          if (next < fields.size())
            attribute(child, "Field", fields[next++]->name);
        }
        value(child, item);
      }
      return;
    }

    throw std::runtime_error("Unsupported deployment value in SMP export");
  }

  void typeHeader(pugi::xml_node xml, const Type& n,
                  std::string_view kind) const {
    named(xml, n);
    attribute(xml, "xsi:type", kind);
    attribute(xml, "Uuid", tag(n, "uuid"));
    for (auto modifier : n.modifiers)
      if (isVisibilityModifier(modifier))
        attribute(xml, "Visibility", modifier);
  }

  // A `Type` element, by the kind of `n`.
  void type(pugi::xml_node parent, const Type& n) const {
    auto xml = parent.append_child("Type");
    if (auto* v = as<PrimitiveType>(&n)) {
      typeHeader(xml, *v, "Types:PrimitiveType");
      return;
    }

    if (auto* v = as<NativeType>(&n)) {
      typeHeader(xml, *v, "Types:NativeType");
      // Each `@platform` opens a mapping, with the tags up to the next one. Any
      // tag before the first one belongs to the first one.
      const auto names = docTagRanges(*v, "platform");
      for (std::size_t i = 0; i < std::max<std::size_t>(names.size(), 1); ++i) {
        const auto begin = i == 0 ? 0 : names[i].begin;
        const auto end = i + 1 < names.size()
                             ? names[i + 1].begin
                             : std::numeric_limits<std::uint32_t>::max();
        const auto mapped = [&](std::string_view tagName) {
          for (auto& found : docTagRanges(*v, tagName))
            if (found.begin >= begin && found.begin < end)
              return std::move(found.content);
          return std::string{};
        };
        auto platform = xml.append_child("Platform");
        attribute(platform, "Type", mapped("type"));
        const auto name = names.empty() ? std::string{} : names[i].content;
        attribute(platform, "Name", name.empty() ? "cpp" : name);
        for (const auto& [attributeName, tagName] :
             {std::pair{"Namespace", "namespace"},
              std::pair{"Location", "location"}})
          if (const auto value = mapped(tagName); !value.empty())
            attribute(platform, attributeName, value);
      }
      return;
    }

    if (auto* v = as<Enumeration>(&n)) {
      typeHeader(xml, *v, "Types:Enumeration");
      for (auto* literal : v->literal) {
        auto item = xml.append_child("Literal");
        named(item, *literal);
        auto numeric = getValue(literal->value);
        attribute(item, "Value",
                  numeric ? std::to_string(
                                static_cast<std::int64_t>(numeric->integral))
                          : "0");
      }
      return;
    }

    // A bound, a length or a size as an attribute. A bound has the kind of its
    // type.
    const auto exprAttr = [&](const char* key, const Expression* expr) {
      if (!expr) return;
      const auto expected =
          as<Integer>(&n) || as<Float>(&n) ? getPTK(&n) : PTK::Int64;
      auto v = getValueAs(expr, expected);
      if (!v) throw std::runtime_error("Cannot evaluate SMP type bound");
      attribute(xml, key, numberText(*v));
    };

    if (auto* v = as<Integer>(&n)) {
      typeHeader(xml, *v, "Types:Integer");
      if (v->primitiveType) ref(xml, "PrimitiveType", *v->primitiveType);
      exprAttr("Minimum", v->minimum);
      exprAttr("Maximum", v->maximum);
      auto unit = tag(*v, "unit");
      if (!unit.empty()) attribute(xml, "Unit", unit);
      return;
    }

    if (auto* v = as<Float>(&n)) {
      typeHeader(xml, *v, "Types:Float");
      if (v->primitiveType) ref(xml, "PrimitiveType", *v->primitiveType);
      exprAttr("Minimum", v->minimum);
      exprAttr("Maximum", v->maximum);
      if (!v->range.empty()) {
        attribute(xml, "MinInclusive",
                  v->range == "..." || v->range == "..<" ? "true" : "false");
        attribute(xml, "MaxInclusive",
                  v->range == "..." || v->range == "<.." ? "true" : "false");
      }
      auto unit = tag(*v, "unit");
      if (!unit.empty()) attribute(xml, "Unit", unit);
      return;
    }

    if (auto* v = as<String>(&n)) {
      typeHeader(xml, *v, "Types:String");
      exprAttr("Length", v->length);
      return;
    }

    if (auto* v = as<Array>(&n)) {
      typeHeader(xml, *v, "Types:Array");
      ref(xml, "ItemType", v->itemType);
      exprAttr("Size", v->size);
      return;
    }

    if (auto* v = as<ValueReference>(&n)) {
      typeHeader(xml, *v, "Types:ValueReference");
      ref(xml, "Type", v->type);
      return;
    }

    if (auto* v = as<AttributeType>(&n)) {
      typeHeader(xml, *v, "Types:AttributeType");
      ref(xml, "Type", v->type);
      if (!v->default_)
        throw std::runtime_error("The attribute type '" + id(*v)
                                 + "' has no default value, which SMP XML "
                                   "requires");
      expression(xml.append_child("Default"), v->default_, v->type.get());
      attribute(xml, "AllowMultiple",
                hasTag(*v, "allowMultiple") ? "true" : "false");
      for (const auto& usage : docTagRanges(*v, "usage"))
        element(xml, "Usage", usage.content);
      return;
    }

    if (auto* v = as<EventType>(&n)) {
      typeHeader(xml, *v, "Catalogue:EventType");
      if (v->eventArgs) ref(xml, "EventArgs", *v->eventArgs);
      return;
    }

    if (auto* v = as<Component>(&n)) {
      typeHeader(xml, *v,
                 as<Service>(v) ? "Catalogue:Service" : "Catalogue:Model");
      if (v->base) ref(xml, "Base", *v->base);
      for (const auto& base : v->interface_) ref(xml, "Interface", base);
      for (auto* m : v->elements) member(xml, *m);
      return;
    }

    if (auto* v = as<Interface>(&n)) {
      typeHeader(xml, *v, "Catalogue:Interface");
      for (const auto& base : v->base) ref(xml, "Base", base);
      for (auto* m : v->elements) member(xml, *m);
      return;
    }

    if (auto* v = as<Structure>(&n)) {
      typeHeader(xml, *v,
                 as<Exception>(v) ? "Types:Exception"
                 : as<Class>(v)   ? "Types:Class"
                                  : "Types:Structure");
      if (auto* cls = as<Class>(v); cls && cls->base)
        ref(xml, "Base", *cls->base);
      for (auto modifier : v->modifiers)
        if (modifier == "abstract") attribute(xml, "Abstract", "true");
      for (auto* m : v->elements) member(xml, *m);
      return;
    }

    throw std::runtime_error("Unsupported catalogue type in SMP export");
  }

  // A member of a type: the element named after its kind, with what that kind
  // has.
  void member(pugi::xml_node parent, const NamedElement& n) const {
    std::string tagName;
    if (as<Constant>(&n))
      tagName = "Constant";
    else if (as<Field>(&n))
      tagName = "Field";
    else if (as<Property>(&n))
      tagName = "Property";
    else if (as<Operation>(&n))
      tagName = "Operation";
    else if (as<Association>(&n))
      tagName = "Association";
    else if (as<Container>(&n))
      tagName = "Container";
    else if (as<Reference>(&n))
      tagName = "Reference";
    else if (as<EntryPoint>(&n))
      tagName = "EntryPoint";
    else if (as<EventSink>(&n))
      tagName = "EventSink";
    else if (as<EventSource>(&n))
      tagName = "EventSource";
    else
      throw std::runtime_error("Unsupported catalogue member in SMP export");

    auto xml = parent.append_child(tagName.c_str());
    named(xml, n);
    if (auto* v = as<VisibilityElement>(&n))
      for (auto modifier : v->modifiers)
        if (isVisibilityModifier(modifier))
          attribute(xml, "Visibility", modifier);

    if (auto* v = as<Constant>(&n)) {
      ref(xml, "Type", v->type);
      valueChild(xml, "Value", v->value, v->type.get());
    }

    if (auto* v = as<Field>(&n)) {
      ref(xml, "Type", v->type);
      valueChild(xml, "Default", v->default_, v->type.get());
      for (auto m : v->modifiers) {
        if (m == "input") attribute(xml, "Input", "true");
        if (m == "output") attribute(xml, "Output", "true");
        if (m == "transient") attribute(xml, "State", "false");
      }
    }

    if (auto* v = as<Property>(&n)) {
      ref(xml, "Type", v->type);
      if (v->attachedField) ref(xml, "AttachedField", *v->attachedField);
      for (const auto& e : v->getRaises) ref(xml, "GetRaises", e);
      for (const auto& e : v->setRaises) ref(xml, "SetRaises", e);
      for (auto m : v->modifiers)
        if (isAccessKind(m)) attribute(xml, "Access", m);
      auto category = tag(*v, "category");
      if (!category.empty()) attribute(xml, "Category", category);
    }

    if (auto* v = as<Association>(&n)) ref(xml, "Type", v->type);

    if (auto* v = as<Operation>(&n)) {
      for (auto* p : v->parameter) {
        auto item = xml.append_child("Parameter");
        named(item, *p);
        ref(item, "Type", p->type);
        if (!p->direction.empty()) attribute(item, "Direction", p->direction);
        // ECSS SMP 2025 holds the default as C++ text, 2020 as a typed value.
        // C++ text has its own escapes, and holds nothing that XML cannot.
        if (p->default_ && standard == "ECSS_SMP_2025")
          item.append_child("Default").text().set(
              cppExpression(p->default_, p->type.get()).c_str());
        else
          valueChild(item, "Default", p->default_, p->type.get());
      }
      if (auto* p = v->returnParameter) {
        auto item = xml.append_child("Parameter");
        named(item, *p);
        attribute(item, "Direction", "return");
        ref(item, "Type", p->type);
      }
      for (const auto& e : v->raisedException) ref(xml, "RaisedException", e);
    }

    if (auto* v = as<NamedElementWithMultiplicity>(&n)) {
      attribute(xml, "Lower", std::to_string(getLower(*v).value_or(1)));
      attribute(xml, "Upper", std::to_string(getUpper(*v).value_or(1)));
    }

    if (auto* v = as<Container>(&n)) {
      ref(xml, "Type", v->type);
      if (v->defaultComponent)
        ref(xml, "DefaultComponent", *v->defaultComponent);
    }

    // The link is `Interface` in ECSS SMP 2020, and `Type` in 2025.
    if (auto* v = as<Reference>(&n))
      ref(xml, standard == "ECSS_SMP_2020" ? "Interface" : "Type",
          v->interface_);

    if (auto* v = as<EntryPoint>(&n)) {
      for (const auto& e : v->input) ref(xml, "Input", e);
      for (const auto& e : v->output) ref(xml, "Output", e);
    }

    if (auto* v = as<EventSink>(&n)) ref(xml, "Type", v->type);
    if (auto* v = as<EventSource>(&n)) {
      ref(xml, "Type", v->type);
      if (hasTag(*v, "singlecast")) attribute(xml, "Multicast", "false");
    }
  }

  void namespaceNode(pugi::xml_node parent, const Namespace& n) const {
    auto xml = parent.append_child("Namespace");
    named(xml, n);
    for (auto* item : n.elements) {
      if (auto* ns = as<Namespace>(item))
        namespaceNode(xml, *ns);
      else if (auto* t = as<Type>(item))
        type(xml, *t);
    }
  }

  // The template parameters of an assembly or a schedule.
  void parameters(pugi::xml_node xml,
                  const std::vector<TemplateParameter*>& values) const {
    for (auto* parameter : values) {
      auto item = xml.append_child("Parameter");
      attribute(item, "Name", parameter->name);
      if (auto* p = as<StringParameter>(parameter)) {
        attribute(item, "xsi:type", "Assembly:StringArgument");
        if (p->value.data()) attribute(item, "Value", p->value);
      }
      if (auto* p = as<Int32Parameter>(parameter)) {
        attribute(item, "xsi:type", "Assembly:Int32Argument");
        if (p->value) attribute(item, "Value", std::to_string(*p->value));
      }
    }
  }

  void arguments(pugi::xml_node xml,
                 const std::vector<TemplateArgument*>& values) const {
    for (auto* argument : values) {
      auto item = xml.append_child("Argument");
      attribute(item, "Name", argument->parameter.refText);
      if (auto* p = as<StringArgument>(argument)) {
        attribute(item, "xsi:type", "Assembly:StringArgument");
        attribute(item, "Value", p->value);
      }
      if (auto* p = as<Int32Argument>(argument)) {
        attribute(item, "xsi:type", "Assembly:Int32Argument");
        attribute(item, "Value", std::to_string(p->value));
      }
    }
  }

  void callParameters(pugi::xml_node xml,
                      const std::vector<ParameterValue*>& values) const {
    for (auto* p : values) {
      auto item = xml.append_child("Parameter");
      attribute(item, "Parameter", p->parameter);
      value(item.append_child("Value"), p->value);
    }
  }

  // A link. An interface link only has a source path, which the XML splits into
  // `OwnerPath` and `Reference`.
  void link(pugi::xml_node xml, const Link& v) const {
    auto item = xml.append_child("Link");
    if (auto* n = as<InterfaceLink>(&v)) {
      attribute(item, "xsi:type", "LinkBase:InterfaceLink");
      const auto source = path(n->sourcePath);
      const auto last = source.find_last_of("./");
      element(item, "OwnerPath",
              last == source.npos                  ? "."
              : last == 0 && source.front() == '/' ? "/"
                                                   : source.substr(0, last));
      element(item, "Reference",
              last == source.npos ? source : source.substr(last + 1));
      if (n->backReference)
        element(item, "BackReference", n->backReference->reference.refText);
    } else {
      attribute(
          item, "xsi:type",
          as<FieldLink>(&v) ? "LinkBase:FieldLink" : "LinkBase:EventLink");
      element(item, "OwnerPath", path(v.ownerPath));
    }
    element(item, "ClientPath", path(v.clientPath));
  }

  // The content shared by a configuration, its components and the assembly
  // elements.
  void configurationElements(pugi::xml_node xml,
                             const std::vector<AstNode*>& values) const {
    for (auto* node : values) {
      if (auto* n = as<FieldValue>(node))
        value(xml.append_child("FieldValue"), n);
      else if (auto* n = as<ConfigurationUsage>(node)) {
        auto item = xml.append_child("Include");
        if (n->path) attribute(item, "Path", path(n->path));
        ref(item, "Configuration", n->configuration);
      } else if (auto* n = as<ComponentConfiguration>(node)) {
        auto item = xml.append_child("Component");
        attribute(item, "Path", path(n->name));
        configurationElements(item, n->elements);
      } else if (auto* n = as<GlobalEventHandler>(node)) {
        auto item = xml.append_child("GlobalEventHandler");
        attribute(item, "EntryPointName",
                  n->entryPoint ? n->entryPoint->reference.refText
                                : std::string_view{});
        attribute(item, "GlobalEventName", n->globalEventName);
      } else if (auto* n = as<OperationCall>(node)) {
        auto item = xml.append_child("Invocation");
        attribute(item, "xsi:type", "Assembly:OperationCall");
        attribute(item, "Operation",
                  n->operation ? n->operation->reference.refText
                               : std::string_view{});
        callParameters(item, n->parameters);
      } else if (auto* n = as<PropertyValue>(node)) {
        auto item = xml.append_child("Invocation");
        attribute(item, "xsi:type", "Assembly:PropertyValue");
        attribute(
            item, "Property",
            n->property ? n->property->reference.refText : std::string_view{});
        value(item.append_child("Value"), n->value);
      }
    }
  }

  void assemblyConfiguration(pugi::xml_node xml,
                             const AssemblyComponentConfiguration& v,
                             const char* tag = "ComponentConfiguration") const {
    auto item = xml.append_child(tag);
    attribute(item, "InstancePath", path(v.name));
    configurationElements(item, v.elements);
  }

  // A model instance. The level-2 schemas have no `Id` here.
  void model(pugi::xml_node xml, const ModelInstance& n,
             std::string_view container = {}) const {
    auto item = xml.append_child("Model");
    named(item, n);
    item.remove_attribute("Id");
    // The doc comment of a sub-instance stands before `container += name`.
    if (const auto* sub = as<SubInstance>(n.parent)) describe(item, *sub);
    if (!container.empty()) attribute(item, "Container", container);

    // An instance without implementation takes the default component of its
    // container.
    const auto* implementation = n.implementation.get();
    if (!implementation)
      if (auto* sub = as<SubInstance>(n.parent); sub && sub->container)
        if (auto* c = as<Container>(sub->container->reference.get());
            c && c->defaultComponent)
          implementation = as<Component>(c->defaultComponent->get());
    // `Implementation` is written as a C++ name, with `::`.
    const auto impl =
        cppName(n.strImplementation && !n.strImplementation->empty()
                    ? *n.strImplementation
                : implementation ? qualifiedName(*implementation)
                                 : std::string(n.implementation.refText));
    if (!impl.empty()) attribute(item, "Implementation", impl);

    configurationElements(item, n.elements);
    for (auto* e : n.elements) {
      if (auto* l = as<Link>(e)) link(item, *l);
      if (auto* sub = as<SubInstance>(e); sub && sub->instance) {
        const auto slot = sub->container ? sub->container->reference.refText
                                         : std::string_view{};
        if (auto* child = as<ModelInstance>(sub->instance))
          model(item, *child, slot);
        else if (auto* child = as<AssemblyInstance>(sub->instance)) {
          auto instance = item.append_child("Assembly");
          named(instance, *child);
          instance.remove_attribute("Id");
          // The doc comment stands before `container += name`.
          describe(instance, *sub);
          attribute(instance, "Container", slot);
          // Another document is named by its XML file name.
          const auto documentReference = [&](const char* tag,
                                             const auto& reference) {
            if (const auto* target = reference.get())
              element(instance, tag, xmlFileName(*target->getDocument()));
          };
          documentReference("Assembly", child->assembly);
          arguments(instance, child->arguments);
          documentReference("Configuration", child->configuration);
          documentReference("LinkBase", child->linkBase);
          for (auto* c : child->elements)
            assemblyConfiguration(instance, *c, "ModelConfiguration");
        }
      }
    }
  }

  void componentLinks(pugi::xml_node xml, const ComponentLinkBase& n) const {
    auto item = xml.append_child("Component");
    attribute(item, "Path", path(n.name));
    for (auto* e : n.elements) {
      if (auto* l = as<Link>(e)) link(item, *l);
      if (auto* c = as<ComponentLinkBase>(e)) componentLinks(item, *c);
    }
  }

  // The name of the kind of a schedule node, which `schedule` numbers.
  std::string generatedKind(const AstNode* node) const {
    if (as<CallOperation>(node)) return "CallOperation";
    if (as<SetProperty>(node)) return "SetProperty";
    if (as<Transfer>(node)) return "Transfer";
    if (as<Trigger>(node)) return "Trigger";
    if (as<EmitGlobalEvent>(node)) return "EmitGlobalEvent";
    if (as<ExecuteTask>(node)) return "ExecuteTask";
    if (as<MissionEvent>(node)) return "MissionEvent";
    if (as<EpochEvent>(node)) return "EpochEvent";
    if (as<SimulationEvent>(node)) return "SimulationEvent";
    if (as<ZuluEvent>(node)) return "ZuluEvent";
    if (as<GlobalEventTriggeredEvent>(node)) return "GlobalEventTriggeredEvent";
    throw std::runtime_error("Unsupported schedule node in SMP export");
  }

  // Activities and events have no name in XSMP: each gets its kind and a
  // number.
  void schedule(pugi::xml_node xml, const Schedule& n) const {
    parameters(xml, n.parameters);
    if (!n.epochTime.empty()) element(xml, "EpochTime", n.epochTime);
    if (!n.missionStart.empty()) element(xml, "MissionStart", n.missionStart);
    std::map<std::string, std::size_t> eventIndices;
    for (auto* e : n.elements) {
      if (auto* task = as<Task>(e)) {
        auto t = xml.append_child("Task");
        named(t, *task);
        std::map<std::string, std::size_t> activityIndices;
        for (auto* activity : task->elements) {
          const auto kind = generatedKind(activity);
          const auto name = kind + std::to_string(++activityIndices[kind]);
          auto a = t.append_child("Activity");
          attribute(a, "Id", id(*task) + "." + name);
          attribute(a, "Name", name);
          describe(a, *activity);
          if (auto* v = as<CallOperation>(activity)) {
            attribute(a, "xsi:type", "Schedule:CallOperation");
            element(a, "OperationPath", path(v->operationPath));
            callParameters(a, v->parameters);
          } else if (auto* v = as<SetProperty>(activity)) {
            attribute(a, "xsi:type", "Schedule:SetProperty");
            element(a, "PropertyPath", path(v->propertyPath));
            value(a.append_child("Value"), v->value);
          } else if (auto* v = as<Transfer>(activity)) {
            attribute(a, "xsi:type", "Schedule:Transfer");
            element(a, "OutputFieldPath", path(v->outputFieldPath));
            element(a, "InputFieldPath", path(v->inputFieldPath));
          } else if (auto* v = as<Trigger>(activity)) {
            attribute(a, "xsi:type", "Schedule:Trigger");
            element(a, "EntryPoint", path(v->entryPoint));
          } else if (auto* v = as<EmitGlobalEvent>(activity)) {
            attribute(a, "xsi:type", "Schedule:EmitGlobalEvent");
            element(a, "EventName", v->eventName);
            element(a, "synchronous", v->asynchronous ? "false" : "true");
          } else if (auto* v = as<ExecuteTask>(activity)) {
            attribute(a, "xsi:type", "Schedule:ExecuteTask");
            ref(a, "Task", v->task);
            arguments(a, v->parameter);
            if (v->root) attribute(a, "Root", path(v->root));
          }
        }
      } else if (auto* event = as<Event>(e)) {
        const auto kind = generatedKind(event);
        const auto name = kind + std::to_string(++eventIndices[kind]);
        auto item = xml.append_child("Event");
        // No name holds `-`: a task cannot have this `Id`.
        attribute(item, "Id", id(n) + "-" + name);
        attribute(item, "Name", name);
        describe(item, *event);
        ref(item, "Task", event->task);
        attribute(item, "CycleTime", event->cycleTime);
        if (event->repeatCount)
          attribute(item, "RepeatCount", std::to_string(*event->repeatCount));

        if (auto* v = as<MissionEvent>(event)) {
          attribute(item, "xsi:type", "Schedule:MissionEvent");
          attribute(item, "MissionTime", v->missionTime);
        }
        if (auto* v = as<EpochEvent>(event)) {
          attribute(item, "xsi:type", "Schedule:EpochEvent");
          attribute(item, "EpochTime", v->epochTime);
        }
        if (auto* v = as<SimulationEvent>(event)) {
          attribute(item, "xsi:type", "Schedule:SimulationEvent");
          attribute(item, "SimulationTime", v->simulationTime);
        }
        if (auto* v = as<ZuluEvent>(event)) {
          attribute(item, "xsi:type", "Schedule:ZuluEvent");
          attribute(item, "ZuluTime", v->zuluTime);
        }
        if (auto* v = as<GlobalEventTriggeredEvent>(event)) {
          attribute(item, "xsi:type", "Schedule:GlobalEventTriggeredEvent");
          attribute(item, "StartEvent", v->startEvent);
          if (!v->stopEvent.empty()) attribute(item, "StopEvent", v->stopEvent);
          if (!v->delay.empty()) attribute(item, "Delay", v->delay);
          if (!v->timeKind.empty()) {
            // `mission` gives `MissionTime`, and so on.
            auto time = std::string(v->timeKind);
            time.front() =
                std::toupper(static_cast<unsigned char>(time.front()));
            attribute(item, "TimeKind", time + "Time");
          }
        }
      }
    }
  }
};

// Moves the children of each element into the order that the SMDL schemas
// expect, as `Writer` does not write them in it. A child that the list does not
// name goes last, and the others keep their relative order.
void canonicalXmlOrder(pugi::xml_node node) {
  std::vector<pugi::xml_node> children;
  for (auto child : node.children())
    if (child.type() == pugi::node_element) {
      canonicalXmlOrder(child);
      children.push_back(child);
    }
  const std::string name = node.name();
  const std::string type = node.attribute("xsi:type").value();

  // Every element starts with these two, then what its kind adds.
  std::vector<std::string_view> order{"Description", "Metadata"};
  const auto extend = [&](std::initializer_list<std::string_view> names) {
    order.insert(order.end(), names.begin(), names.end());
  };
  if (name == "Catalogue:Catalogue" || name == "Namespace")
    extend({"Namespace", "Type"});
  else if (type == "Catalogue:Model" || type == "Catalogue:Service")
    extend({"Constant", "Property", "Operation", "Base", "Interface",
            "EntryPoint", "EventSink", "EventSource", "Field", "Association",
            "Container", "Reference"});
  else if (type == "Catalogue:Interface")
    extend({"Constant", "Property", "Operation", "Base"});
  else if (type == "Types:Class" || type == "Types:Exception")
    extend(
        {"Constant", "Field", "Base", "Property", "Operation", "Association"});
  else if (type == "Types:Structure")
    extend({"Constant", "Field"});
  else if (type == "Types:AttributeType")
    extend({"Type", "Default", "Usage"});
  else if (type == "Types:Enumeration")
    extend({"Literal"});
  else if (type == "Types:Array")
    extend({"ItemType"});
  else if (type == "Types:Integer" || type == "Types:Float")
    extend({"PrimitiveType"});
  else if (type == "Types:NativeType")
    extend({"Platform"});
  else if (name == "Constant")
    extend({"Type", "Value"});
  else if (name == "Field" || name == "Parameter")
    extend({"Type", "Default"});
  else if (name == "Property")
    extend({"Type", "AttachedField", "GetRaises", "SetRaises"});
  else if (name == "Operation")
    extend({"Parameter", "RaisedException"});
  else if (name == "Association" || name == "Reference" || name == "EventSink"
           || name == "EventSource")
    extend({"Type", "Interface"});
  else if (name == "Container")
    extend({"Type", "DefaultComponent"});
  else if (name == "EntryPoint")
    extend({"Input", "Output"});
  // A `Component` of a link base holds something else than one of a
  // configuration.
  else if (name == "Component" && [&] {
             auto root = node;
             while (root.parent()
                    && root.parent().type() != pugi::node_document)
               root = root.parent();
             return std::string_view(root.name()) == "LinkBase:LinkBase";
           }())
    extend({"Link", "Component"});
  else if (name == "Configuration:Configuration" || name == "Component")
    extend({"Include", "Component", "Link", "FieldValue"});
  else if (name == "Assembly:Assembly")
    extend({"ComponentConfiguration", "Parameter", "Model"});
  else if (name == "Model")
    extend({"Assembly", "Model", "Link", "FieldValue", "Invocation",
            "GlobalEventHandler"});
  else if (name == "Assembly")
    extend({"Assembly", "Argument", "ModelConfiguration", "Configuration",
            "LinkBase"});
  else if (name == "ComponentConfiguration" || name == "ModelConfiguration")
    extend({"FieldValue", "Invocation", "GlobalEventHandler"});
  else if (name == "Schedule:Schedule")
    extend({"Parameter", "EpochTime", "MissionStart", "Task", "Event"});
  else if (name == "Task")
    extend({"Activity"});
  else if (type == "Schedule:CallOperation" || type == "Assembly:OperationCall")
    extend({"OperationPath", "Parameter"});
  else if (type == "Schedule:SetProperty" || type == "Assembly:PropertyValue")
    extend({"PropertyPath", "Value"});
  else if (type == "Schedule:ExecuteTask")
    extend({"Task", "Argument"});
  else if (type == "Schedule:Transfer")
    extend({"OutputFieldPath", "InputFieldPath"});
  else if (type == "Schedule:EmitGlobalEvent")
    extend({"EventName", "synchronous"});
  else if (type == "LinkBase:InterfaceLink")
    extend({"OwnerPath", "ClientPath", "Reference", "BackReference"});
  else if (type == "LinkBase:FieldLink" || type == "LinkBase:EventLink")
    extend({"OwnerPath", "ClientPath"});
  else if (name == "Metadata")
    extend({"Type", "Value"});

  const auto rank = [&](pugi::xml_node child) {
    const auto at = std::ranges::find(order, std::string_view(child.name()));
    return at == order.end() ? order.size()
                             : static_cast<std::size_t>(at - order.begin());
  };
  std::ranges::stable_sort(children,
                           [&](auto a, auto b) { return rank(a) < rank(b); });
  for (auto child : children) node.append_move(child);
}

// The name of the root element of a document and the extension of its XML file.
std::pair<std::string, std::string> rootKind(const AstNode* node) {
  if (as<Catalogue>(node)) return {"Catalogue", ".smpcat"};
  if (as<Configuration>(node)) return {"Configuration", ".smpcfg"};
  if (as<Assembly>(node)) return {"Assembly", ".smpasb"};
  if (as<LinkBase>(node)) return {"LinkBase", ".smplnk"};
  if (as<Schedule>(node)) return {"Schedule", ".smpsed"};
  throw std::runtime_error("Document is not an SMP modeling document");
}

}  // namespace

// `standard`, or else the one of the project of `document`, or else 2020.
static std::string resolvedStandard(const cursiv::Document& document,
                                    std::string_view standard) {
  if (!standard.empty()) return std::string(standard);
  if (document.language)
    if (auto* shared = dynamic_cast<const XsmpSharedCoreServices*>(
            &document.language->shared());
        shared && shared->projectManager)
      if (auto* project = shared->projectManager->getProject(document))
        return std::string(project->standard);
  return "ECSS_SMP_2020";
}

std::string exportDocument(const cursiv::Document& document,
                           std::string_view standard) {
  const auto revision = resolvedStandard(document, standard);
  standard = revision;
  if (standard != "ECSS_SMP_2020" && standard != "ECSS_SMP_2025")
    throw std::invalid_argument("Unknown SMP standard revision");

  const auto* ast = document.parseResult.value;
  const auto [kind, extension] = rootKind(ast);

  pugi::xml_document xml;
  auto declaration = xml.append_child(pugi::node_declaration);
  attribute(declaration, "version", "1.0");
  attribute(declaration, "encoding", "UTF-8");
  copyrightNotice(xml, document);
  auto root = xml.append_child((kind + ":" + kind).c_str());

  // An Assembly, a LinkBase or a Schedule uses the 2019 core under either
  // standard, and its own namespace is always 2025.
  const auto level2 =
      kind == "Assembly" || kind == "LinkBase" || kind == "Schedule";
  const auto base =
      "http://www.ecss.nl/smp/"
      + std::string(standard == "ECSS_SMP_2020" || level2 ? "2019" : "2025");
  attribute(root, "xmlns:Elements", base + "/Core/Elements");
  attribute(root, "xmlns:Types", base + "/Core/Types");
  for (const auto* ns :
       {"Catalogue", "Configuration", "LinkBase", "Assembly", "Schedule"})
    attribute(root, (std::string("xmlns:") + ns).c_str(),
              (std::string(ns) == "LinkBase" || std::string(ns) == "Assembly"
                       || std::string(ns) == "Schedule"
                   ? "http://www.ecss.nl/smp/2025"
                   : base)
                  + "/Smdl/" + ns);
  attribute(root, "xmlns:xsd", "http://www.w3.org/2001/XMLSchema");
  attribute(root, "xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
  attribute(root, "xmlns:xlink", "http://www.w3.org/1999/xlink");

  Writer writer(document, standard);
  writer.metadata(root, *ast);
  if (auto* n = as<Catalogue>(ast))
    for (auto* ns : n->elements) writer.namespaceNode(root, *ns);
  else if (auto* n = as<Configuration>(ast))
    writer.configurationElements(root, n->elements);
  else if (auto* n = as<Assembly>(ast)) {
    writer.parameters(root, n->parameters);
    for (auto* cfg : n->configurations)
      writer.assemblyConfiguration(root, *cfg);
    if (n->model) writer.model(root, *n->model);
  } else if (auto* n = as<LinkBase>(ast))
    for (auto* component : n->elements) writer.componentLinks(root, *component);
  else if (auto* n = as<Schedule>(ast))
    writer.schedule(root, *n);

  canonicalXmlOrder(root);
  std::ostringstream stream;
  xml.save(stream, "  ", pugi::format_default, pugi::encoding_utf8);
  auto result = std::move(stream).str();
  // pugixml leaves a carriage return raw in the text of an element, where a
  // reader would take it for a line feed.
  for (auto at = result.find('\r'); at != result.npos;
       at = result.find('\r', at + 5))
    result.replace(at, 1, "&#13;");
  return result;
}

// The `.smppkg` descriptor of a catalogue: an `Implementation` per type, except
// an Interface, and a `Dependency` per other catalogue it refers to, except a
// builtin library or a catalogue only reached through an Interface.
static std::string packageDocument(const cursiv::Document& document,
                                   std::string_view standard) {
  const auto* catalogue = as<Catalogue>(document.parseResult.value);
  if (!catalogue)
    throw std::invalid_argument(
        "Only a catalogue has an SMP package descriptor");

  Writer writer(document, standard);
  pugi::xml_document xml;
  auto declaration = xml.append_child(pugi::node_declaration);
  attribute(declaration, "version", "1.0");
  attribute(declaration, "encoding", "UTF-8");
  copyrightNotice(xml, document);

  auto root = xml.append_child("Package:Package");
  const auto base =
      "http://www.ecss.nl/smp/"
      + std::string(standard == "ECSS_SMP_2020" ? "2019" : "2025");
  attribute(root, "xmlns:Package", base + "/Smdl/Package");
  attribute(root, "xmlns:Elements", base + "/Core/Elements");
  attribute(root, "xmlns:Types", base + "/Core/Types");
  attribute(root, "xmlns:Catalogue", base + "/Smdl/Catalogue");
  attribute(root, "xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
  attribute(root, "xmlns:xlink", "http://www.w3.org/1999/xlink");
  writer.metadata(root, *catalogue);

  std::filesystem::path catalogueFile(cursiv::uri_file_name(document.uri));
  catalogueFile.replace_extension(".smpcat");
  for (const auto* node : document.parseResult.nodes())
    if (const auto* type = as<Type>(node); type && !as<Interface>(type)) {
      auto implementation = root.append_child("Implementation");
      attribute(implementation, "xlink:title", type->name);
      attribute(implementation, "xlink:href",
                catalogueFile.string() + "#" + writer.id(*type));
    }

  for (const auto* dependency : packageDependencies(*catalogue)) {
    auto item = root.append_child("Dependency");
    attribute(item, "xlink:title", dependency->name);
    std::filesystem::path file(
        cursiv::uri_file_name(dependency->getDocument()->uri));
    file.replace_extension(".smppkg");
    attribute(item, "xlink:href", file.string() + "#" + writer.id(*dependency));
  }

  std::ostringstream output;
  xml.save(output, "  ", pugi::format_default, pugi::encoding_utf8);
  return output.str();
}

std::filesystem::path generateDocument(const cursiv::Document& document,
                                       const std::filesystem::path& directory,
                                       std::string_view standard) {
  const auto revision = resolvedStandard(document, standard);
  standard = revision;
  const auto [kind, extension] = rootKind(document.parseResult.value);
  const auto generatedDirectory = directory / "smdl-gen";
  auto output = generatedDirectory
                / std::filesystem::path(cursiv::uri_file_name(document.uri));
  output.replace_extension(extension);

  const auto content = exportDocument(document, standard);
  const auto package = as<Catalogue>(document.parseResult.value)
                           ? packageDocument(document, standard)
                           : std::string{};

  writeFileIfChanged(output, content);
  if (!package.empty()) {
    auto descriptor = output;
    descriptor.replace_extension(".smppkg");
    writeFileIfChanged(descriptor, package);
  }
  return output;
}

}  // namespace xsmp
