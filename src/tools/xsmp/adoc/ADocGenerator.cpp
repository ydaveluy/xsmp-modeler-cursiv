#include "ADocGenerator.hpp"

#include <algorithm>
#include <cctype>
#include <cursiv/syntax-tree/CstUtils.hpp>
#include <format>
#include <optional>
#include <string>
#include <utility>
#include <vector>
#include <xsmp/AttributeHelper.hpp>
#include <xsmp/DeploymentSemantics.hpp>
#include <xsmp/solver/Solver.hpp>
#include <xsmp/utils/FileWriter.hpp>
#include <xsmp/utils/TreeUtils.hpp>
#include <xsmp/utils/Utf8.hpp>
#include <xsmp/utils/XsmpUtils.hpp>

namespace xsmp {

namespace {

using cursiv::AstNode;

template <class T>
const T* as(const AstNode* node) {
  return cursiv::ast_cast<T>(node);
}

// The generated text, built one line at a time. The end of each line is
// trimmed.
class Lines {
 public:
  // Adds each line of `text`. An empty text is one empty line.
  void add(std::string_view text) { add("", text); }

  // Adds each line of `text` after `prefix`. The next lines are indented as
  // wide as the prefix.
  void add(std::string_view prefix, std::string_view text) {
    std::string margin(prefix);
    for (std::size_t from = 0;;) {
      const auto end = text.find('\n', from);
      line(margin + std::string(text.substr(from, end - from)));
      if (end == std::string_view::npos) return;
      from = end + 1;
      margin.assign(prefix.size(), ' ');
    }
  }

  void blank() { add(""); }

  [[nodiscard]] bool empty() const { return !_started; }

  [[nodiscard]] const std::string& str() const { return _text; }

 private:
  void line(std::string_view text) {
    while (!text.empty()
           && std::isspace(static_cast<unsigned char>(text.back())))
      text.remove_suffix(1);
    if (_started) _text += '\n';
    _text += text;
    _started = true;
  }

  std::string _text;
  bool _started = false;
};

using InfoRows = std::vector<std::pair<std::string_view, std::string_view>>;

std::string join(const std::vector<std::string>& values,
                 std::string_view separator = "\n") {
  std::string result;
  for (std::size_t i = 0; i < values.size(); ++i) {
    if (i) result += separator;
    result += values[i];
  }
  return result;
}

// The parts that are not empty, separated by `separator`.
std::string joinNonEmpty(std::initializer_list<std::string_view> parts,
                         std::string_view separator) {
  std::string result;
  for (const auto part : parts) {
    if (part.empty()) continue;
    if (!result.empty()) result += separator;
    result += part;
  }
  return result;
}

// `value` as the text of a table cell: on one line, with the `|` escaped.
std::string cell(std::string_view value) {
  std::string result;
  for (auto c : value) {
    if (c == '|') result += '\\';
    result += c == '\n' ? ' ' : c;
  }
  return result;
}

// Whether AsciiDoc shows `text` as it is: letters, digits, and signs that only
// mean something doubled or before a sign that is not one of them.
bool isPlainText(std::string_view text) {
  static constexpr std::string_view signs = " .,:-\\";
  return std::ranges::all_of(text,
                             [](const char c) {
                               const auto byte = static_cast<unsigned char>(c);
                               return byte >= 0x80 || std::isalnum(byte)
                                      || signs.find(c) != signs.npos;
                             })
         && text.find("..") == text.npos && text.find("--") == text.npos;
}

// `text` in a passthrough, which keeps AsciiDoc from reading `*x*`, `<<x>>`,
// `&amp;` or `image:x[]` as markup. Only a `]` is escaped there.
std::string passthrough(std::string_view text) {
  std::string result = "pass:c[";
  for (const auto c : text) {
    if (c == ']') result += '\\';
    result += c;
  }
  return result + ']';
}

// `value` as a literal between its `delimiter`s.
std::string literal(std::string_view value, char delimiter = '"') {
  return delimiter + cppEscaped(value, delimiter) + delimiter;
}

// `value` as a literal that a table shows as written.
std::string shownLiteral(std::string_view value, char delimiter = '"') {
  const auto text = literal(value, delimiter);
  return isPlainText(std::string_view(text).substr(1, text.size() - 2))
             ? text
             : passthrough(text);
}

std::string fqn(const AstNode& node, std::string_view separator = "::") {
  auto text = qualifiedName(node);
  std::string result;
  for (auto c : text)
    if (c == '.')
      result += separator;
    else
      result += c;
  return result;
}

// The name of the AST type of `node`. A ReturnParameter counts as a Parameter.
std::string kind(const AstNode& node) {
  std::string result(node.type ? node.type->name : "Element");
  if (result == "ReturnParameter") return "Parameter";
  return result;
}

std::string lower(std::string value) {
  std::ranges::transform(value, value.begin(),
                         [](unsigned char c) { return std::tolower(c); });
  return value;
}

// The text of a path, with its `unsafe` keyword when `unsafe` is true.
std::string path(const Path* value, bool unsafe = true) {
  if (!value) return {};
  const auto segment = [](const PathSegment* node) -> std::string_view {
    if (auto* n = as<ConcretePathNamedSegment>(node))
      return n->reference.refText;
    if (auto* n = as<PatternPathNamedSegment>(node)) return n->text;
    if (as<PathParentSegment>(node)) return "..";
    if (as<PathSelfSegment>(node)) return ".";
    return {};
  };
  std::string text = unsafe && value->unsafe ? "unsafe " : "";
  if (value->absolute) text += '/';
  text += segment(value->head);
  for (auto* item : value->elements)
    if (auto* member = as<PathMember>(item)) {
      text += member->separator;
      text += segment(member->segment);
    } else if (auto* index = as<PathIndex>(item))
      text += '[' + std::to_string(index->index) + ']';
  return text;
}

// The name that a local reference gives, with its `unsafe` keyword.
std::string local(const LocalNamedReference* reference) {
  return reference ? (reference->unsafe ? "unsafe " : "")
                         + std::string(reference->reference.refText)
                   : std::string{};
}

// The target of a reference, which can be of any of `Types`.
template <class... Types>
const AstNode* refNode(const cursiv::reference<Types...>& reference) {
  const AstNode* result = nullptr;
  ((result =
        result ? result
               : static_cast<const AstNode*>(reference.template get<Types>())),
   ...);
  return result;
}

// The name of the target, qualified on request, or else the written name.
template <class... Types>
std::string refName(const cursiv::reference<Types...>& reference,
                    bool qualified = false) {
  if (const auto* target = refNode(reference)) {
    if (qualified) return fqn(*target);
    if (const auto* named = as<NamedNode>(target))
      return std::string(named->name);
  }
  return std::string(reference.refText);
}

template <class Type, class Range>
std::vector<const Type*> filter(const Range& elements) {
  std::vector<const Type*> result;
  for (const auto* node : elements)
    if (const auto* typed = as<Type>(node)) result.push_back(typed);

  return result;
}

std::string visibleName(Visibility visibility) {
  return visibility == Visibility::Public      ? "public"
         : visibility == Visibility::Protected ? "protected"
                                               : "private";
}

// The members of a Structure, a Component or an Interface.
std::vector<const NamedElement*> body(const Type& type) {
  if (auto* node = as<Structure>(&type))
    return {node->elements.begin(), node->elements.end()};
  if (auto* node = as<Component>(&type))
    return {node->elements.begin(), node->elements.end()};
  if (auto* node = as<Interface>(&type))
    return {node->elements.begin(), node->elements.end()};
  return {};
}

// ---------------------------------------------------------------------------
// Pieces shared by the documents
// ---------------------------------------------------------------------------

// A heading and an empty line.
void heading(Lines& out, int level, std::string_view title) {
  out.add(std::string(std::clamp(level, 1, 6), '=') + ' ', title);
  out.blank();
}

// ` |text`, which adds a column to a row of cells written on one line.
std::string inlineCell(bool enabled, std::string_view text) {
  return enabled ? std::format(" |{}", text) : "";
}

// `<<anchor,label>>`
std::string crossLink(std::string_view anchor, std::string_view label) {
  return std::format("<<{},{}>>", anchor, label);
}

// A link to a type whose anchor is its name.
std::string fixedCross(std::string_view name) {
  std::string anchor(name);
  for (std::size_t at = 0; (at = anchor.find("::", at)) != std::string::npos;)
    anchor.replace(at, 2, "-");
  return crossLink(anchor, name);
}

// A link to what `reference` names. The label is the qualified name, unless
// the target is in the same document as `context`.
template <class... Types>
std::string cross(const cursiv::reference<Types...>& reference,
                  const AstNode& context) {
  const auto* target = refNode(reference);
  if (!target) return std::string(reference.refText);
  const auto* named = as<NamedNode>(target);
  return crossLink(fqn(*target, "-"),
                   target->getDocument() == context.getDocument() && named
                       ? std::string(named->name)
                       : fqn(*target));
}

// The description of `node` in a `++++` passthrough block, if it has one.
void descriptionBlock(Lines& out, const AstNode& node) {
  const auto text = docDescription(node);
  if (!text || text->empty()) return;
  out.add("++++");
  out.add(*text);
  out.add("++++");
}

std::string descriptionCell(const AstNode& node) {
  return cell(docDescription(node).value_or(""));
}

// Whether one element has a description: only then a table has that column.
template <class Range>
bool described(const Range& elements) {
  return std::ranges::any_of(
      elements, [](const auto* n) { return docDescription(*n).has_value(); });
}

// A table row is written as one `|cell` line for each cell.
void row(Lines& rows, std::initializer_list<std::string_view> cells) {
  for (const auto text : cells) rows.add("|" + std::string(text));
}

// A table with a caption and a line of column titles. `rows` come from `row`.
// A table without rows leaves one empty line.
void table(Lines& out, std::string_view caption, std::string_view columns,
           const Lines& rows) {
  out.add(std::format(".{}", caption));
  out.add("[%autowidth.stretch]");
  out.add("|===");
  out.add(columns);
  out.blank();
  out.add(rows.str());
  out.add("|===");
}

// The table of the information of an element. A row without value is left out.
// `emptyLine` writes an empty line when no row is left.
void infoTable(Lines& out, std::string_view name, const InfoRows& rows,
               bool emptyLine = false) {
  out.add(std::format(".{}'s information", name));
  out.add("[%autowidth.stretch]");
  out.add("|===");
  auto any = false;
  for (const auto& [label, value] : rows) {
    if (value.empty()) continue;
    out.add(std::format(".^h|{} |{}", label, cell(value)));
    any = true;
  }
  if (!any && emptyLine) out.blank();
  out.add("|===");
}

// The heading of an element, its description and its information table, then
// an empty line.
void elementHeader(Lines& out, int level, std::string_view title,
                   const AstNode& node, std::string_view infoTitle,
                   const InfoRows& rows, bool emptyLine = false) {
  heading(out, level, title);
  descriptionBlock(out, node);
  out.blank();
  infoTable(out, infoTitle, rows, emptyLine);
  out.blank();
}

// ---------------------------------------------------------------------------
// Values
// ---------------------------------------------------------------------------

// A list of values in brackets. A leading `[[` would start an anchor, so it is
// escaped.
std::string bracketList(const std::vector<std::string>& values) {
  auto text = "[" + join(values, ", ") + "]";
  return text.starts_with("[[") ? "\\" + text : text;
}

// The value of an expression on one line, or nothing if it has none. It goes in
// a table unless `plain` is set.
std::optional<std::string> shortValue(const Expression* expression,
                                      bool plain = false) {
  if (!expression) return std::nullopt;
  if (const auto* collection = as<CollectionLiteral>(expression)) {
    std::vector<std::string> values;
    for (auto* child : collection->elements) {
      const auto* initializer = as<DesignatedInitializer>(child);
      values.push_back(
          shortValue(initializer ? initializer->expr : child, plain)
              .value_or("?"));
    }
    return bracketList(values);
  }
  if (const auto* keyword = as<KeywordExpression>(expression))
    return std::string(keyword->name);

  const auto value = getValue(expression);
  if (!value) return std::nullopt;
  if (value->type == PTK::Bool) return value->boolean ? "true" : "false";
  if (value->type == PTK::Enum)
    return value->literal ? std::string(value->literal->name) : "";
  if (value->type == PTK::String8)
    return plain ? literal(value->text) : shownLiteral(value->text);
  if (value->type == PTK::Char8)
    return plain ? literal(value->text, '\'') : shownLiteral(value->text, '\'');
  if (isFloatingType(value->type)) return std::format("{}", value->floating);
  if (isIntegralType(value->type))
    return isSignedIntegral(value->type)
               ? std::to_string(static_cast<std::int64_t>(value->integral))
               : std::to_string(value->integral);
  return std::nullopt;
}

// The value of a deployment document. A float keeps the spelling it has in the
// document.
std::string value(const Value* node) {
  if (!node) return "?";
  if (auto* n = as<BoolValue>(node)) return n->value ? "true" : "false";
  if (auto* n = as<Char8Value>(node)) return shownLiteral(n->value, '\'');
  if (auto* n = as<String8Value>(node)) return shownLiteral(n->value);
#define XSMP_ADOC_TEXT(TYPE) \
  if (auto* n = as<TYPE>(node)) return std::string(n->value)
  XSMP_ADOC_TEXT(DateTimeValue);
  XSMP_ADOC_TEXT(DurationValue);
  XSMP_ADOC_TEXT(IntValue);
#undef XSMP_ADOC_TEXT

  if (as<FloatValue>(node) || as<Float32Value>(node)
      || as<Float64Value>(node)) {
    if (node->cstNodeId != cursiv::kNoNode) {
      const auto& document = *node->getDocument();
      const auto ranges =
          cursiv::find_nodes_for_feature(*node, "value", &document);
      if (const auto first = ranges.begin(); first != ranges.end())
        return std::string(document.textDocument().getText().substr(
            first->begin(), first->end() - first->begin()));
    }
    if (const auto* n = as<FloatValue>(node))
      return std::format("{}", n->value);
    if (const auto* n = as<Float32Value>(node))
      return std::format("{}", n->value);
    return std::format("{}", as<Float64Value>(node)->value);
  }
#define XSMP_ADOC_NUMBER(TYPE) \
  if (auto* n = as<TYPE>(node)) return std::format("{}", n->value)
  XSMP_ADOC_NUMBER(Int8Value);
  XSMP_ADOC_NUMBER(Int16Value);
  XSMP_ADOC_NUMBER(Int32Value);
  XSMP_ADOC_NUMBER(Int64Value);
  XSMP_ADOC_NUMBER(UInt8Value);
  XSMP_ADOC_NUMBER(UInt16Value);
  XSMP_ADOC_NUMBER(UInt32Value);
  XSMP_ADOC_NUMBER(UInt64Value);
#undef XSMP_ADOC_NUMBER

  if (auto* n = as<EnumerationValue>(node)) {
    if (n->reference.get()) return fqn(*n->reference.get());
    return !n->reference.refText.empty() ? std::string(n->reference.refText)
           : n->value                    ? std::to_string(*n->value)
                                         : "";
  }

  if (auto* n = as<ArrayValue>(node)) {
    std::vector<std::string> values;
    for (auto* item : n->elements) values.push_back(value(item));
    return n->startIndex && *n->startIndex != 0
               ? std::format("[{}: {}]", *n->startIndex, join(values, ", "))
               : bracketList(values);
  }

  if (auto* n = as<StructureValue>(node)) {
    std::vector<std::string> values;
    for (auto* item : n->elements) values.push_back(value(item));
    return "{ " + join(values, ", ") + " }";
  }

  if (auto* n = as<CfgStructureFieldValue>(node)) {
    const auto target =
        n->field ? std::string(n->field->reference.refText) : "";
    const auto prefix = n->field && n->field->unsafe ? "unsafe " : "";
    return target.empty()
               ? value(n->value)
               : std::format("{}{} = {}", prefix, target, value(n->value));
  }

  if (auto* n = as<FieldValue>(node)) {
    const auto target = path(n->field);
    return target.empty() ? value(n->value)
                          : std::format("{} = {}", target, value(n->value));
  }

  return "?";
}

template <class Range>
std::string arguments(const Range& range) {
  std::vector<std::string> values;
  for (auto* argument : range) {
    if (auto* n = as<StringArgument>(argument))
      values.push_back(std::format("{} = {}", refName(n->parameter), n->value));
    if (auto* n = as<Int32Argument>(argument))
      values.push_back(std::format("{} = {}", refName(n->parameter), n->value));
  }

  return join(values, ", ");
}

std::string parameters(const std::vector<ParameterValue*>& range) {
  std::vector<std::string> values;
  for (auto* parameter : range)
    values.push_back(
        std::format("{} = {}", parameter->parameter, value(parameter->value)));
  return join(values, ", ");
}

// ---------------------------------------------------------------------------
// Deployment documents
// ---------------------------------------------------------------------------

// `templateParameters`, `includes`, `fieldValues`, `statements` and `links`
// write nothing, not even a heading, without a row.
void templateParameters(Lines& out,
                        const std::vector<TemplateParameter*>& range,
                        int level) {
  if (range.empty()) return;
  Lines rows;
  for (auto* parameter : range) {
    if (auto* n = as<StringParameter>(parameter))
      row(rows, {n->name, "string",
                 cell(isPlainText(n->value) ? std::string(n->value)
                                            : passthrough(n->value))});
    if (auto* n = as<Int32Parameter>(parameter))
      row(rows, {n->name, "int32", n->value ? std::to_string(*n->value) : ""});
  }
  heading(out, level, "Template Parameters");
  table(out, "Template parameters", "|Name |Type |Default", rows);
}

// The heading and the information table of a document, from its doc tags.
void documentHeader(Lines& out, const Document& node,
                    std::string_view documentKind, InfoRows extras = {}) {
  std::vector<std::string> creators;
  for (const auto& tag : docTagRanges(node, "creator"))
    creators.push_back(tag.content);
  const auto creator = join(creators, ", ");
  const auto date = docTag(node, "date").value_or("");
  const auto version = docTag(node, "version").value_or("");
  InfoRows rows{{"Kind", documentKind},
                {"Name", node.name},
                {"Date", date},
                {"Creator", creator},
                {"Version", version}};
  rows.insert(rows.end(), extras.begin(), extras.end());
  elementHeader(out, 1,
                docTag(node, "title")
                    .value_or(std::format("{} {}", documentKind, node.name)),
                node, node.name, rows);
}

template <class Range>
void includes(Lines& out, const Range& elements, int level) {
  const auto values = filter<ConfigurationUsage>(elements);
  if (values.empty()) return;
  const bool hasDescription = described(values);
  Lines rows;
  for (const auto* n : values) {
    row(rows, {refName(n->configuration), path(n->path)});
    if (hasDescription) row(rows, {descriptionCell(*n)});
  }

  heading(out, level, "Includes");
  table(out, "Includes",
        "|Configuration |Path" + inlineCell(hasDescription, "Description"),
        rows);
}

template <class Range>
void fieldValues(Lines& out, const Range& elements, int level) {
  const auto values = filter<FieldValue>(elements);
  if (values.empty()) return;
  const bool hasDescription = described(values);
  Lines rows;
  for (const auto* n : values) {
    row(rows, {path(n->field), cell(value(n->value))});
    if (hasDescription) row(rows, {descriptionCell(*n)});
  }

  heading(out, level, "Field Values");
  table(out, "Field Values",
        "|Field |Value" + inlineCell(hasDescription, "Description"), rows);
}

// The field values, event subscriptions and invocations of a configuration.
template <class Range>
void statements(Lines& out, const Range& elements, int level) {
  std::vector<const AstNode*> values;
  for (auto* element : elements)
    if (as<FieldValue>(element) || as<Invocation>(element)
        || as<GlobalEventHandler>(element))
      values.push_back(element);

  if (values.empty()) return;
  const auto hasDescription = described(values);
  Lines rows;
  for (auto* element : values) {
    std::string kind, target, details;
    if (auto* n = as<FieldValue>(element)) {
      kind = "field";
      target = path(n->field);
      details = value(n->value);
    }
    if (auto* n = as<GlobalEventHandler>(element)) {
      kind = "subscribe";
      target = local(n->entryPoint);
      details = shownLiteral(n->globalEventName);
    }
    if (auto* n = as<OperationCall>(element)) {
      kind = "call";
      target = local(n->operation);
      details = parameters(n->parameters);
    }
    if (auto* n = as<PropertyValue>(element)) {
      kind = "property";
      target = local(n->property);
      details = value(n->value);
    }
    row(rows, {kind, cell(target), cell(details)});
    if (hasDescription) row(rows, {descriptionCell(*element)});
  }

  heading(out, level, "Statements");
  table(out, "Statements",
        "|Kind |Target |Details" + inlineCell(hasDescription, "Description"),
        rows);
}

template <class Range>
void links(Lines& out, const Range& elements, int level) {
  const auto values = filter<Link>(elements);
  if (values.empty()) return;
  const bool hasDescription = described(values);
  const bool back =
      std::ranges::any_of(values, [](auto* n) { return as<InterfaceLink>(n); });

  Lines rows;
  for (auto* n : values) {
    std::string owner = path(n->ownerPath), reference, backReference;
    std::string linkKind = as<FieldLink>(n) ? "field" : "event";
    if (auto* interface = as<InterfaceLink>(n)) {
      // An interface link has only a source path: split it into owner and
      // reference.
      linkKind = "interface";
      const auto* source = interface->sourcePath;
      owner = source && source->absolute ? "/" : ".";
      const PathSegment* last = source ? source->head : nullptr;
      if (source && !source->elements.empty()) {
        auto full = path(source, false);
        if (auto* member = as<PathMember>(source->elements.back())) {
          last = member->segment;
          const auto split = full.find_last_of("./");
          owner = split == std::string::npos ? full : full.substr(0, split);
        } else {
          last = nullptr;
          owner = full.substr(0, full.rfind('['));
        }
        if (owner.empty()) owner = source->absolute ? "/" : ".";
      }
      if (auto* concrete = as<ConcretePathNamedSegment>(last))
        reference = concrete->reference.refText;
      if (auto* pattern = as<PatternPathNamedSegment>(last))
        reference = pattern->text;
      backReference = local(interface->backReference);
    }

    row(rows,
        {linkKind, cell(owner), cell(reference), cell(path(n->clientPath))});
    if (back) row(rows, {cell(backReference)});
    if (hasDescription) row(rows, {descriptionCell(*n)});
  }

  heading(out, level, "Links");
  table(out, "Links",
        "|Kind |Owner |Reference |Client" + inlineCell(back, "Back Reference")
            + inlineCell(hasDescription, "Description"),
        rows);
}

void assemblyConfiguration(Lines& out,
                           const AssemblyComponentConfiguration& node,
                           int level, std::string title = {}) {
  if (title.empty()) title = "Configure " + path(node.name);
  elementHeader(out, level, title, node, title, {{"Path", path(node.name)}});
  statements(out, node.elements, level + 1);
}

void assemblyInstance(Lines& out, const AssemblyInstance& node, int level,
                      std::string_view container) {
  elementHeader(out, level, "Assembly Instance " + std::string(node.name), node,
                node.name,
                {{"Name", node.name},
                 {"Container", container},
                 {"Assembly", refName(node.assembly)},
                 {"Configuration", refName(node.configuration)},
                 {"Link Base", refName(node.linkBase)},
                 {"Arguments", arguments(node.arguments)}},
                /*emptyLine=*/true);
  Lines configurations;
  for (auto* configuration : node.elements)
    assemblyConfiguration(configurations, *configuration, level + 1,
                          "Configuration " + path(configuration->name));
  out.add(configurations.str());
}

// A model instance, with its statements, links and sub-instances.
void model(Lines& out, const ModelInstance& node, int level,
           std::string_view title, std::string_view container = {}) {
  const auto* component = targetType(&node);
  const auto implementation = component && as<Component>(component)
                                  ? fqn(*component)
                              : !node.implementation.refText.empty()
                                  ? std::string(node.implementation.refText)
                                  : node.strImplementation.value_or("");

  Lines subInstances;
  for (auto* sub : filter<SubInstance>(node.elements)) {
    if (auto* child = as<ModelInstance>(sub->instance))
      model(subInstances, *child, level + 1,
            "Model Instance " + std::string(child->name),
            local(sub->container));
    if (auto* child = as<AssemblyInstance>(sub->instance))
      assemblyInstance(subInstances, *child, level + 1, local(sub->container));
  }

  elementHeader(out, level, title, node, node.name,
                {{"Name", node.name},
                 {"Container", container},
                 {"Implementation", implementation}},
                /*emptyLine=*/true);
  statements(out, node.elements, level + 1);
  links(out, node.elements, level + 1);
  out.add(subInstances.str());
}

void componentConfiguration(Lines& out, const ComponentConfiguration& node,
                            int level) {
  const auto title = "Component " + path(node.name);
  elementHeader(
      out, level, title, node, title,
      {{"Path", path(node.name)}, {"Type", refName(node.context, true)}});
  includes(out, node.elements, level + 1);
  fieldValues(out, node.elements, level + 1);
  Lines nested;
  for (auto* n : filter<ComponentConfiguration>(node.elements))
    componentConfiguration(nested, *n, level + 1);
  out.add(nested.str());
}

void componentLinks(Lines& out, const ComponentLinkBase& node, int level) {
  const auto title = "Component " + path(node.name);
  elementHeader(out, level, title, node, title, {{"Path", path(node.name)}});
  links(out, node.elements, level + 1);
  Lines nested;
  for (auto* n : filter<ComponentLinkBase>(node.elements))
    componentLinks(nested, *n, level + 1);
  out.add(nested.str());
}

// A task and its activities. A task on an assembly names the type of its root
// model.
void task(Lines& out, const Task& node, int level) {
  const auto* component = refNode(node.context);
  if (auto* assembly = as<Assembly>(component))
    component = targetType(assembly->model);
  const auto context =
      component ? fqn(*component) : refName(node.context, true);
  const auto hasDescription = described(node.elements);
  Lines activities;
  for (auto* element : node.elements) {
    std::string kind, target, details;
    if (auto* n = as<CallOperation>(element)) {
      kind = "call";
      target = path(n->operationPath);
      details = parameters(n->parameters);
    }
    if (auto* n = as<SetProperty>(element)) {
      kind = "property";
      target = path(n->propertyPath);
      details = value(n->value);
    }
    if (auto* n = as<Transfer>(element)) {
      kind = "transfer";
      target = path(n->outputFieldPath);
      details = "to " + path(n->inputFieldPath);
    }
    if (auto* n = as<Trigger>(element)) {
      kind = "trig";
      target = path(n->entryPoint);
    }
    if (auto* n = as<ExecuteTask>(element)) {
      kind = "execute";
      target = refName(n->task);
      details = joinNonEmpty({path(n->root), arguments(n->parameter)}, " | ");
    }
    if (auto* n = as<EmitGlobalEvent>(element)) {
      kind = "emit";
      target = shownLiteral(n->eventName);
      details = n->asynchronous ? "async" : "sync";
    }

    row(activities, {kind, cell(target), cell(details)});
    if (hasDescription) row(activities, {descriptionCell(*element)});
  }

  elementHeader(out, level, "Task " + std::string(node.name), node, node.name,
                {{"Name", node.name}, {"Component", context}});
  if (activities.empty()) return;
  heading(out, level + 1, "Activities");
  table(out, std::format("{}'s activities", node.name),
        "|Kind |Target |Details" + inlineCell(hasDescription, "Description"),
        activities);
}

// What starts an event: its time, or the global events of a triggered event.
std::string eventAnchor(const Event& node) {
  if (auto* n = as<MissionEvent>(&node))
    return "mission " + std::string(n->missionTime);
  if (auto* n = as<EpochEvent>(&node))
    return "epoch " + std::string(n->epochTime);
  if (auto* n = as<SimulationEvent>(&node))
    return "simulation " + std::string(n->simulationTime);
  if (auto* n = as<ZuluEvent>(&node)) return "zulu " + std::string(n->zuluTime);
  if (auto* n = as<GlobalEventTriggeredEvent>(&node))
    return joinNonEmpty(
        {"on " + shownLiteral(n->startEvent),
         n->stopEvent.empty() ? "" : "until " + shownLiteral(n->stopEvent),
         n->timeKind.empty() ? "" : "using " + std::string(n->timeKind)},
        " ");
  return {};
}

void eventRow(Lines& rows, const Event& event) {
  auto eventKind = kind(event);
  if (auto pos = eventKind.find("Event"); pos != std::string::npos)
    eventKind.replace(pos, 5, " event");
  const auto* triggered = as<GlobalEventTriggeredEvent>(&event);
  row(rows,
      {refName(event.task), cell(lower(eventKind)), cell(eventAnchor(event)),
       triggered ? cell(triggered->delay) : "", cell(event.cycleTime),
       event.repeatCount ? std::to_string(*event.repeatCount) : "",
       descriptionCell(event)});
}

// ---------------------------------------------------------------------------
// Catalogue
// ---------------------------------------------------------------------------

std::optional<std::string> view(const NamedElement& node) {
  const auto kind = getViewKind(node);
  return kind ? std::optional(std::string(*kind)) : std::nullopt;
}

// What a field is: its modifiers, where a field that is not transient counts as
// `state`.
std::string fieldKinds(const Field& field) {
  std::vector<std::string> flags;
  const auto has = [&](std::string_view flag) {
    return std::ranges::find(field.modifiers, flag) != field.modifiers.end();
  };
  if (has("output")) flags.emplace_back("output");
  if (has("input")) flags.emplace_back("input");
  flags.emplace_back(has("transient") ? "transient" : "state");
  for (const auto& [id, flag] : {std::pair{"Attributes.Forcible", "forcible"},
                                 std::pair{"Attributes.Failure", "failure"}})
    if (attributeBoolValue(field, id).value_or(false)) flags.emplace_back(flag);
  return join(flags, ", ");
}

// The `@unit` of the type of a field, when it is an Integer or a Float.
std::optional<std::string> unit(const Field& field) {
  const auto* target = field.type.get();
  return target && (as<Integer>(target) || as<Float>(target))
             ? docTag(*target, "unit")
             : std::nullopt;
}

// A multiplicity in a diagram: `?`, `*`, `+`, one number or a range.
std::string multiplicity(const NamedElementWithMultiplicity& node) {
  const auto lower = getLower(node).value_or(1),
             upper = getUpper(node).value_or(1);
  if (lower == 0 && upper == 1) return "?";
  if (lower == upper) return std::to_string(lower);
  if (upper < 0)
    return lower == 0 ? "*" : lower == 1 ? "+" : std::format("{} .. *", lower);
  return std::format("{} .. {}", lower, upper);
}

// A bound in a table: `*` when unbounded, `?` when unknown.
std::string bound(std::optional<std::int64_t> value) {
  return !value ? "?" : *value < 0 ? "*" : std::to_string(*value);
}

// The heading and the introduction of the members of a type of one kind.
void membersHeading(Lines& out, const Type& type, std::string_view title) {
  out.add(std::format("==== {}", title));
  out.add(std::format("The {} {} provides the following {}:", type.name,
                      lower(kind(type)), lower(std::string(title))));
  out.blank();
}

template <class Member>
std::vector<const Member*> withVisibility(
    const std::vector<const Member*>& members, Visibility visibility) {
  std::vector<const Member*> result;
  for (const auto* member : members)
    if (getRealVisibility(*member) == visibility) result.push_back(member);

  return result;
}

// Members get one table per visibility, in this order. A column exists only
// when a member of that table fills it.
constexpr Visibility visibilities[] = {
    Visibility::Private, Visibility::Protected, Visibility::Public};

void constants(Lines& out, const Type& type) {
  const auto all = filter<Constant>(body(type));
  if (all.empty()) return;
  membersHeading(out, type, "Constants");
  for (const auto visibility : visibilities) {
    const auto members = withVisibility(all, visibility);
    if (members.empty()) continue;
    const auto hasDescription = described(members);
    Lines rows;
    for (const auto* n : members) {
      row(rows, {n->name, cross(n->type, *n),
                 cell(shortValue(n->value).value_or(""))});
      if (hasDescription) row(rows, {descriptionCell(*n)});
    }
    table(out,
          std::format("{}'s {} constants", type.name, visibleName(visibility)),
          "|Name |Type |Value" + inlineCell(hasDescription, "Description"),
          rows);
  }
}

void properties(Lines& out, const Type& type) {
  const auto all = filter<Property>(body(type));
  if (all.empty()) return;
  membersHeading(out, type, "Properties");
  for (const auto visibility : visibilities) {
    const auto members = withVisibility(all, visibility);
    if (members.empty()) continue;
    const auto hasDescription = described(members);
    const auto hasCategory = std::ranges::any_of(
        members, [](auto* n) { return docTag(*n, "category").has_value(); });
    const auto hasView = std::ranges::any_of(
        members, [](auto* n) { return view(*n).has_value(); });
    Lines rows;
    for (const auto* n : members) {
      row(rows, {n->name, cross(n->type, *n)});
      if (hasCategory) row(rows, {cell(docTag(*n, "category").value_or(""))});
      if (hasView) row(rows, {view(*n).value_or("")});
      if (hasDescription) row(rows, {descriptionCell(*n)});
    }
    table(out,
          std::format("{}'s {} properties", type.name, visibleName(visibility)),
          "|Name |Type" + inlineCell(hasCategory, "Category")
              + inlineCell(hasView, "View Kind")
              + inlineCell(hasDescription, "Description"),
          rows);
  }
}

void fields(Lines& out, const Type& type) {
  const auto all = filter<Field>(body(type));
  if (all.empty()) return;
  membersHeading(out, type, "Fields");
  for (const auto visibility : visibilities) {
    const auto members = withVisibility(all, visibility);
    if (members.empty()) continue;
    const auto hasDescription = described(members);
    const auto hasUnit = std::ranges::any_of(
        members, [](auto* n) { return unit(*n).has_value(); });
    const auto hasView = std::ranges::any_of(
        members, [](auto* n) { return view(*n).has_value(); });
    const auto hasInitialValue = std::ranges::any_of(
        members, [](auto* n) { return n->default_ != nullptr; });
    Lines rows;
    for (const auto* n : members) {
      row(rows, {fieldKinds(*n), n->name, cross(n->type, *n)});
      if (hasUnit) row(rows, {cell(unit(*n).value_or(""))});
      if (hasView) row(rows, {view(*n).value_or("")});
      if (hasInitialValue)
        row(rows, {cell(shortValue(n->default_).value_or(""))});
      if (hasDescription) row(rows, {descriptionCell(*n)});
    }
    table(out,
          std::format("{}'s {} fields", type.name, visibleName(visibility)),
          "|Kind |Name |Type" + inlineCell(hasUnit, "Unit")
              + inlineCell(hasView, "View Kind")
              + inlineCell(hasInitialValue, "Initial Value")
              + inlineCell(hasDescription, "Description"),
          rows);
  }
}

void associations(Lines& out, const Type& type) {
  const auto all = filter<Association>(body(type));
  if (all.empty()) return;
  membersHeading(out, type, "Associations");
  for (const auto visibility : visibilities) {
    const auto members = withVisibility(all, visibility);
    // A visibility without association leaves an empty line.
    if (members.empty()) {
      out.blank();
      continue;
    }
    const auto hasDescription = described(members);
    Lines rows;
    for (const auto* n : members) {
      row(rows, {n->name, cross(n->type, *n)});
      if (hasDescription) row(rows, {descriptionCell(*n)});
    }
    table(
        out,
        std::format("{}'s {} associations", type.name, visibleName(visibility)),
        "|Name |Type" + inlineCell(hasDescription, "Description"), rows);
  }
}

// The heading of an Operation, its description and its parameter table.
void operation(Lines& out, const Operation& node) {
  out.add(std::format("===== Operation {}", node.name));
  if (const auto text = docDescription(node)) out.add(*text);
  if (node.parameter.empty() && !node.returnParameter) return;
  const auto defaults = std::ranges::any_of(
      node.parameter, [](auto* n) { return n->default_ != nullptr; });
  const auto hasDescription = described(node.parameter);
  Lines parameterRows;
  for (auto* n : node.parameter)
    parameterRows.add(std::format(
        "|{} |{} |{}{}{}", n->direction.empty() ? "in" : n->direction, n->name,
        cross(n->type, *n),
        inlineCell(defaults, cell(shortValue(n->default_).value_or(""))),
        inlineCell(hasDescription, descriptionCell(*n))));
  // Without parameter or without return value, an empty line stays in place.
  Lines rows;
  rows.add(parameterRows.str());
  if (const auto* n = node.returnParameter)
    rows.add(std::format("|return |{} |{}{}{}",
                         n->name.empty() ? "return" : n->name,
                         cross(n->type, *n), inlineCell(defaults, ""),
                         inlineCell(hasDescription, descriptionCell(*n))));
  else
    rows.blank();
  table(out, std::format("{}'s parameters", node.name),
        "|Direction |Name |Type" + inlineCell(defaults, "Default Value")
            + inlineCell(hasDescription, "Description"),
        rows);
}

void operations(Lines& out, const Type& type) {
  const auto members = filter<Operation>(body(type));
  if (members.empty()) return;
  membersHeading(out, type, "Operations");
  for (const auto* n : members) operation(out, *n);
}

void entryPoints(Lines& out, const Type& type) {
  const auto members = filter<EntryPoint>(body(type));
  if (members.empty()) return;
  const auto hasDescription = described(members);
  const auto hasView = std::ranges::any_of(
      members, [](auto* n) { return view(*n).has_value(); });
  Lines rows;
  for (const auto* n : members) {
    row(rows, {n->name});
    if (hasView) row(rows, {view(*n).value_or("")});
    if (hasDescription) row(rows, {descriptionCell(*n)});
  }
  membersHeading(out, type, "Entry Points");
  table(out, std::format("{}'s Entry Points", type.name),
        "|Name" + inlineCell(hasView, "View Kind")
            + inlineCell(hasDescription, "Description"),
        rows);
}

template <class Member>
void eventMembers(Lines& out, const Type& type, std::string_view title) {
  const auto members = filter<Member>(body(type));
  if (members.empty()) return;
  const bool hasDescription = described(members);
  Lines rows;
  for (const auto* n : members) {
    row(rows, {n->name, cross(n->type, *n)});
    if (hasDescription) row(rows, {descriptionCell(*n)});
  }

  membersHeading(out, type, title);
  table(out, std::format("{}'s {}", type.name, title),
        "|Name |Type" + inlineCell(hasDescription, "Description"), rows);
}

// The containers or the references of a type.
template <class Member>
void multiplicities(Lines& out, const Type& type, std::string_view title) {
  const auto members = filter<Member>(body(type));
  if (members.empty()) return;
  const bool hasDescription = described(members);
  Lines rows;
  for (const auto* n : members) {
    if constexpr (std::is_same_v<Member, Container>)
      row(rows, {n->name, cross(n->type, *n), bound(getLower(*n)),
                 bound(getUpper(*n))});
    else
      row(rows, {n->name, cross(n->interface_, *n), bound(getLower(*n)),
                 bound(getUpper(*n))});
    if (hasDescription) row(rows, {descriptionCell(*n)});
  }

  membersHeading(out, type, title);
  table(out, std::format("{}'s {}", type.name, lower(std::string(title))),
        "|Name |Type |Minimum |Maximum"
            + inlineCell(hasDescription, "Description"),
        rows);
}

// The literals of an enumeration, or the members of a type that has some.
void content(Lines& out, const Type& type) {
  if (const auto* enumeration = as<Enumeration>(&type)) {
    Lines rows;
    for (auto* n : enumeration->literal)
      rows.add(std::format("|{}|{}", n->name,
                           cell(shortValue(n->value).value_or("?"))));
    out.add(std::format(".{}'s literals", type.name));
    out.add("[%autowidth.stretch]");
    out.add("|===");
    out.add("|Name |Value");
    out.add(rows.str());
    out.add("|===");
    return;
  }
  if (!as<Structure>(&type) && !as<Interface>(&type) && !as<Component>(&type))
    return;
  Lines sections;
  constants(sections, type);
  properties(sections, type);
  operations(sections, type);
  entryPoints(sections, type);
  eventMembers<EventSink>(sections, type, "Event Sinks");
  eventMembers<EventSource>(sections, type, "Event Sources");
  fields(sections, type);
  associations(sections, type);
  multiplicities<Container>(sections, type, "Containers");
  multiplicities<Reference>(sections, type, "References");
  // A type without members leaves an empty line.
  out.add(sections.str());
}

// Class diagrams

// A class in a diagram that shows only its name and its stereotype.
std::string classBox(std::string_view name, std::string_view kind) {
  return std::format("class {} {{ <<{}>> }}", name, kind);
}

// Opens a Mermaid class diagram. The caller closes it with a `....` line.
void diagramStart(Lines& out) {
  out.add("[.center]");
  out.add("[mermaid]");
  out.add("....");
  out.add("classDiagram");
}

std::string mermaidVisibility(const NamedElement& node) {
  switch (getRealVisibility(node)) {
    case Visibility::Public:
      return "+";
    case Visibility::Protected:
      return "#";
    default:
      return "-";
  }
}

// `value` in the line of a class box. Mermaid gives these characters a meaning
// there, and reads `#nnn;` as the character of that number.
std::string mermaidText(std::string_view value) {
  std::string result;
  for (const auto c : value)
    if (std::string_view("{}()~<>#$").find(c) == std::string_view::npos)
      result += c;
    else
      result += std::format("#{};", static_cast<int>(c));
  return result;
}

// The line of a member in a class box.
std::string mermaidMember(const NamedElement& node) {
  if (auto* n = as<Field>(&node))
    return std::format("{}{} {}", mermaidVisibility(node), refName(n->type),
                       n->name);
  if (auto* n = as<EventSource>(&node))
    return std::format("+EventSource~{}~ {}", refName(n->type), n->name);
  if (auto* n = as<EventSink>(&node))
    return std::format("+EventSink~{}~ {}", refName(n->type), n->name);
  if (as<EntryPoint>(&node)) return std::format("+EntryPoint {}", node.name);
  if (auto* n = as<Constant>(&node))
    return std::format("{}constexpr {} {} = {}", mermaidVisibility(node),
                       refName(n->type), n->name,
                       mermaidText(shortValue(n->value, true).value_or("?")));
  if (auto* n = as<Association>(&node))
    return std::format("{}{}* {}", mermaidVisibility(node), refName(n->type),
                       n->name);
  if (auto* n = as<Operation>(&node)) {
    std::vector<std::string> args;
    for (auto* p : n->parameter)
      args.push_back(std::format("{} {}",
                                 p->direction.empty() ? "in" : p->direction,
                                 refName(p->type)));
    return std::format(
        "{}{}({}) {}", mermaidVisibility(node), node.name, join(args, ", "),
        n->returnParameter ? refName(n->returnParameter->type) : "void");
  }
  return {};
}

// Adds the lines of `items`, or one empty line without item. Over `limit`
// items, only the first `limit - 2` stay, then `marker`. The first item and the
// marker of a cut list are indented, the other items are not.
void attributeLines(Lines& out, std::vector<std::string> items,
                    std::size_t limit, std::string_view marker) {
  constexpr std::string_view indent = "                ";
  if (items.size() <= limit) {
    out.add(join(items));
    return;
  }
  items.resize(limit - 2);
  out.blank();
  out.add(indent, items.front());
  for (std::size_t i = 1; i < items.size(); ++i) out.add(items[i]);
  out.add(indent, marker);
  out.blank();
}

// The members and then the operations of a type, in the box of its class.
void attributes(Lines& out, const Type& type) {
  std::vector<std::string> memberLines, operationLines;
  for (auto* n : body(type)) {
    if (as<Operation>(n))
      operationLines.push_back(mermaidMember(*n));
    else if (as<Constant>(n) || as<Field>(n) || as<EventSink>(n)
             || as<EventSource>(n) || as<EntryPoint>(n) || as<Association>(n))
      memberLines.push_back(mermaidMember(*n));
  }

  // Each list can take the lines that the other leaves out of 10, and always 5.
  const auto memberLimit =
      std::max(5, 10 - static_cast<int>(operationLines.size()));
  const auto operationLimit =
      std::max(5, 10 - static_cast<int>(memberLines.size()));
  attributeLines(out, memberLines, memberLimit, "...");
  attributeLines(out, operationLines, operationLimit, "...()");
}

// The class box of `type` with its attributes.
void classWithAttributes(Lines& out, const Type& type) {
  Lines lines;
  attributes(lines, type);
  out.add(std::format("    class {} {{", type.name));
  out.add("        ", lines.str());
  out.add("    }");
}

// The diagram of a component. An unresolved link leaves an empty line.
void mermaidComponent(Lines& out, const Component& node) {
  const auto* base = node.base ? node.base->get() : nullptr;
  Lines interfaces, references, containers, inheritance, referenceRelations,
      containerRelations;
  for (const auto& reference : node.interface_) {
    const auto* target = reference.get();
    interfaces.add(target ? classBox(target->name, kind(*target)) : "");
    inheritance.add(
        target ? std::format("{} <|.. {} : implements", target->name, node.name)
               : "");
  }
  for (auto* reference : filter<Reference>(node.elements)) {
    const auto* target = reference->interface_.get();
    references.add(target ? classBox(target->name, kind(*target)) : "");
    referenceRelations.add(target
                               ? std::format("{} \"{}\" o-- {} : {}", node.name,
                                             multiplicity(*reference),
                                             target->name, reference->name)
                               : "");
  }
  for (auto* container : filter<Container>(node.elements)) {
    const auto* target = container->type.get();
    containers.add(target ? classBox(target->name, kind(*target)) : "");
    containerRelations.add(target
                               ? std::format("{} \"{}\" *-- {} : {}", node.name,
                                             multiplicity(*container),
                                             target->name, container->name)
                               : "");
  }

  diagramStart(out);
  out.add("    ", base ? classBox(base->name, kind(*base)) : "");
  for (const auto* group : {&interfaces, &references, &containers})
    out.add("    ", group->str());
  classWithAttributes(out, node);
  if (base)
    out.add(std::format("    {} <|-- {} : extends", base->name, node.name));
  for (const auto* group :
       {&inheritance, &referenceRelations, &containerRelations})
    out.add("    ", group->str());
}

void mermaidInterface(Lines& out, const Interface& node) {
  Lines bases, relations;
  for (const auto& base : node.base) {
    bases.add(classBox(refName(base), "Interface"));
    relations.add(
        std::format("{} <|-- {} : extends", refName(base), node.name));
  }
  diagramStart(out);
  out.add("    ", bases.str());
  out.blank();
  classWithAttributes(out, node);
  out.blank();
  out.add("    ", relations.str());
}

void mermaidStructure(Lines& out, const Structure& node) {
  const auto* clazz = as<Class>(&node);
  const auto* base = clazz && clazz->base ? clazz->base->get() : nullptr;
  diagramStart(out);
  out.add("    ", base ? classBox(base->name, kind(*base)) : "");
  out.blank();
  classWithAttributes(out, node);
  out.blank();
  out.add("    ", base ? std::format("{} <|-- {}", base->name, node.name) : "");
}

// The diagram of a component, an interface or a structure.
void mermaid(Lines& out, const Type& type) {
  const auto* component = as<Component>(&type);
  const auto* interface = as<Interface>(&type);
  const auto* structure = as<Structure>(&type);
  if (!component && !interface && !structure) return;
  heading(out, 4, "Diagram");
  if (component)
    mermaidComponent(out, *component);
  else if (interface)
    mermaidInterface(out, *interface);
  else
    mermaidStructure(out, *structure);
  out.add("....");
}

// The row `label |<<a>> |<<b>>` of the references of a type.
template <class References>
void referenceInfo(Lines& out, std::string_view label,
                   const References& references, const AstNode& context) {
  if (references.empty()) return;
  std::vector<std::string> values;
  for (const auto& reference : references)
    values.push_back("|" + cross(reference, context));

  out.add(std::format(".{}+.^h|{} {}", references.size(), label,
                      join(values, " ")));
}

// The rows of the information of a type.
void typeInfo(Lines& out, const Type& type) {
  out.add(
      std::format(".^h|Visibility |{}", visibleName(getRealVisibility(type))));
  out.add(std::format(".^h|Qualified Name |{}", fqn(type)));
  out.add(".^h|UUID |", docTag(type, "uuid").value_or(""));
  const auto unitTag = docTag(type, "unit");
  if (auto* n = as<Integer>(&type)) {
    out.add(".^h|Primitive Type |"
            + (n->primitiveType ? cross(*n->primitiveType, type)
                                : fixedCross("Smp::Int32")));
    if (n->minimum)
      out.add(".^h|Minimum |" + shortValue(n->minimum).value_or(""));
    if (n->maximum)
      out.add(".^h|Maximum |" + shortValue(n->maximum).value_or(""));
    if (unitTag) out.add(".^h|Unit |" + *unitTag);
  } else if (auto* n = as<Float>(&type)) {
    out.add(".^h|Primitive Type |"
            + (n->primitiveType ? cross(*n->primitiveType, type)
                                : fixedCross("Smp::Float64")));
    if (n->minimum)
      out.add(std::format(
          ".^h|Minimum |{}{}", shortValue(n->minimum).value_or(""),
          n->range == "<.." || n->range == "<.<" ? " (exclusive)" : ""));
    if (n->maximum)
      out.add(std::format(
          ".^h|Maximum |{}{}", shortValue(n->maximum).value_or(""),
          n->range == "..<" || n->range == "<.<" ? " (exclusive)" : ""));
    if (unitTag) out.add(".^h|Unit |" + *unitTag);
  } else if (auto* n = as<Array>(&type)) {
    out.add(".^h|Item Type |" + cross(n->itemType, type));
    out.add(".^h|Size |" + shortValue(n->size).value_or(""));
  } else if (auto* n = as<EventType>(&type)) {
    out.add(".^h|Event Type |"
            + (n->eventArgs ? cross(*n->eventArgs, type) : fixedCross("void")));
  } else if (as<NativeType>(&type)) {
    const auto nativeType = docTag(type, "type").value_or("");
    out.add(".^h|Type |", crossLink(nativeType, nativeType));
    if (const auto location = docTag(type, "location"))
      out.add(".^h|Location |" + *location);
    if (const auto name = docTag(type, "namespace"))
      out.add(".^h|Namespace |" + *name);
  } else if (auto* n = as<String>(&type)) {
    out.add(".^h|Length |" + shortValue(n->length).value_or(""));
  } else if (auto* n = as<Exception>(&type)) {
    out.add(".^h|Extends |"
            + (n->base ? cross(*n->base, type) : fixedCross("Smp::Exception")));
  } else if (auto* n = as<Class>(&type)) {
    if (n->base) out.add(".^h|Extends |" + cross(*n->base, type));
  } else if (auto* n = as<Interface>(&type)) {
    referenceInfo(out, "Extends", n->base, type);
  } else if (auto* n = as<Component>(&type)) {
    if (n->base) out.add(".^h|Extends |" + cross(*n->base, type));
    referenceInfo(out, "Implements", n->interface_, type);
  }
}

// The section of a type. Its anchor is what `cross` links to.
void type(Lines& out, const Type& node) {
  out.add(std::format("[#{}]", fqn(node, "-")));
  heading(out, 3, std::format("{} {}", kind(node), node.name));
  descriptionBlock(out, node);
  out.blank();
  out.add(std::format(".{}'s informations", node.name));
  out.add("[%autowidth.stretch]");
  out.add("|===");
  typeInfo(out, node);
  out.add("|===");
  out.blank();
  mermaid(out, node);
  out.blank();
  content(out, node);
}

// The diagram of the types of a namespace. A namespace with more than 8 types
// shows the first 6 and a marker for the others. The first class and the marker
// are indented, the other classes are not.
void namespaceDiagram(Lines& out, std::string_view name,
                      const std::vector<const Type*>& types) {
  if (types.empty()) return;
  const auto more = types.size() > 8;
  const auto shown = more ? 6 : types.size();
  constexpr std::string_view indent = "                ";
  diagramStart(out);
  out.blank();
  out.add("direction LR");
  out.blank();
  out.add(std::format("namespace {} {{", name));
  out.blank();
  for (std::size_t i = 0; i < shown; ++i)
    out.add(i == 0 ? indent : "", classBox(types[i]->name, kind(*types[i])));
  if (more) out.add(indent, "class - { <<...>> }");
  out.blank();
  out.add("}");
  out.add("....");
}

// The section of a namespace, with its diagram, nested namespaces and types.
void namespaceDoc(Lines& out, const Namespace& node) {
  Lines elements;
  for (auto* element : node.elements) {
    if (auto* n = as<Namespace>(element))
      namespaceDoc(elements, *n);
    else if (auto* n = as<Type>(element))
      type(elements, *n);
  }
  heading(out, 2, "Namespace " + fqn(node));
  descriptionBlock(out, node);
  namespaceDiagram(out, node.name, filter<Type>(node.elements));
  out.blank();
  out.add(elements.str());
}

// ---------------------------------------------------------------------------
// Documents
// ---------------------------------------------------------------------------

void catalogueDocument(Lines& out, const Catalogue& node) {
  heading(
      out, 1,
      docTag(node, "title").value_or(std::format("Catalogue {}", node.name)));
  descriptionBlock(out, node);
  out.blank();
  Lines namespaces;
  for (auto* ns : node.elements) namespaceDoc(namespaces, *ns);
  out.add(namespaces.str());
}

void configurationDocument(Lines& out, const Configuration& node) {
  documentHeader(out, node, "Configuration");
  includes(out, node.elements, 2);
  const auto components = filter<ComponentConfiguration>(node.elements);
  if (components.empty()) return;
  heading(out, 2, "Component Configurations");
  for (const auto* component : components)
    componentConfiguration(out, *component, 3);
}

void assemblyDocument(Lines& out, const Assembly& node) {
  documentHeader(out, node, "Assembly");
  templateParameters(out, node.parameters, 2);
  if (!node.configurations.empty()) {
    heading(out, 2, "Configure Blocks");
    for (auto* configuration : node.configurations)
      assemblyConfiguration(out, *configuration, 3);
  }
  out.blank();
  heading(out, 2, "Root Model Instance");
  if (node.model)
    model(out, *node.model, 3,
          "Model Instance " + std::string(node.model->name));
  else
    out.blank();
}

void linkBaseDocument(Lines& out, const LinkBase& node) {
  documentHeader(out, node, "Link Base",
                 {{"Assembly", refName(node.assembly)}});
  if (node.elements.empty()) return;
  heading(out, 2, "Component Link Blocks");
  for (auto* component : node.elements) componentLinks(out, *component, 3);
}

void scheduleDocument(Lines& out, const Schedule& node) {
  Lines tasks, events;
  for (auto* element : node.elements) {
    if (auto* t = as<Task>(element)) task(tasks, *t, 3);
    if (auto* event = as<Event>(element)) eventRow(events, *event);
  }
  documentHeader(
      out, node, "Schedule",
      {{"Epoch", node.epochTime}, {"Mission Start", node.missionStart}});
  templateParameters(out, node.parameters, 2);
  if (!tasks.empty()) {
    heading(out, 2, "Tasks");
    out.add(tasks.str());
  }
  if (!events.empty()) {
    heading(out, 2, "Events");
    table(out, "Events",
          "|Task |Kind |Anchor |Delay |Cycle |Repeat |Description", events);
  }
}

std::string renderDocument(const Document& node) {
  Lines out;
  if (auto* n = as<Catalogue>(&node))
    catalogueDocument(out, *n);
  else if (auto* n = as<Configuration>(&node))
    configurationDocument(out, *n);
  else if (auto* n = as<Assembly>(&node))
    assemblyDocument(out, *n);
  else if (auto* n = as<LinkBase>(&node))
    linkBaseDocument(out, *n);
  else if (auto* n = as<Schedule>(&node))
    scheduleDocument(out, *n);
  return out.str();
}

}  // namespace

std::string ADocGenerator::render(const cursiv::AstNode& node) const {
  const auto* document = as<Document>(&node);
  return document ? utf8Text(renderDocument(*document)) : "";
}

void ADocGenerator::generate(const cursiv::AstNode& node,
                             const std::filesystem::path& destination) const {
  if (const auto* document = as<Document>(&node))
    writeFileIfChanged(
        destination / "adoc-gen" / (std::string(document->name) + "-gen.adoc"),
        render(node));
}

}  // namespace xsmp
