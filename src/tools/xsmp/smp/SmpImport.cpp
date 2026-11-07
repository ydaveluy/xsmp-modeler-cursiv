#include <algorithm>
#include <cctype>
#include <cursiv/services/CoreServicesFor.hpp>
#include <cursiv/services/ServiceRegistry.hpp>
#include <cursiv/utils/UriUtils.hpp>
#include <cursiv/workspace/DocumentFactory.hpp>
#include <cursiv/workspace/Documents.hpp>
#include <format>
#include <map>
#include <memory>
#include <pugixml.hpp>
#include <set>
#include <sstream>
#include <stdexcept>
#include <unordered_map>
#include <xsmp/Builtins.hpp>
#include <xsmp/SharedCoreServices.hpp>
#include <xsmp/ast.hpp>
#include <xsmp/generated/Xsmpasb/Module.hpp>
#include <xsmp/generated/Xsmpcat/Module.hpp>
#include <xsmp/generated/Xsmpcfg/Module.hpp>
#include <xsmp/generated/Xsmplnk/Module.hpp>
#include <xsmp/generated/Xsmpsed/Module.hpp>
#include <xsmp/references/QualifiedName.hpp>
#include <xsmp/smp/SmpService.hpp>
#include <xsmp/smp/SmpXmlText.hpp>
#include <xsmp/utils/FileWriter.hpp>
#include <xsmp/utils/XsmpDocumentation.hpp>
#include <xsmp/utils/XsmpUtils.hpp>

namespace xsmp {

namespace {

// `name` without its namespace prefix.
std::string local(std::string_view name) {
  const auto colon = name.find(':');
  return std::string(name.substr(colon == name.npos ? 0 : colon + 1));
}

// The attribute `name`, with or without a prefix, or else `fallback`.
std::string attr(pugi::xml_node node, std::string_view name,
                 std::string fallback = {}) {
  for (auto a : node.attributes())
    if (a.name() == name || local(a.name()) == name)
      return decodeXmlText(a.value());
  return fallback;
}

// The first child of that local name.
pugi::xml_node child(pugi::xml_node node, std::string_view name) {
  for (auto c : node.children())
    if (local(c.name()) == name) return c;
  return {};
}

std::string text(pugi::xml_node node, std::string_view name,
                 std::string fallback = {}) {
  auto c = child(node, name);
  return c ? decodeXmlText(c.text().get()) : std::move(fallback);
}

// A value written as a child element or as an attribute.
std::string field(pugi::xml_node node, std::string_view name,
                  std::string fallback = {}) {
  if (const auto c = child(node, name)) return decodeXmlText(c.text().get());
  const auto a = attr(node, name);
  return a.empty() ? std::move(fallback) : a;
}

std::string lower(std::string value) {
  std::ranges::transform(value, value.begin(),
                         [](unsigned char c) { return std::tolower(c); });
  return value;
}

// The local part of the `xsi:type` of `node`.
std::string kind(pugi::xml_node node) { return local(attr(node, "type")); }

// An `xs:boolean` attribute. Throws on any other value.
bool boolean(pugi::xml_node node, std::string_view key, bool fallback = false) {
  for (const auto a : node.attributes())
    if (a.name() == key || local(a.name()) == key) {
      const std::string_view value = a.value();
      if (value == "true" || value == "1") return true;
      if (value == "false" || value == "0") return false;
      throw std::runtime_error("Invalid XML Schema boolean value '"
                               + std::string(value) + "' for attribute '"
                               + std::string(key) + "'.");
    }
  return fallback;
}

// Whether `root` is one of the five SMP documents, by its local name, which its
// namespace must also end with.
bool supportedDocument(pugi::xml_node root) {
  const std::string_view name = root.name();
  const auto colon = name.find(':');
  const auto declaration = colon == name.npos
                               ? std::string("xmlns")
                               : "xmlns:" + std::string(name.substr(0, colon));
  const std::string_view ns = root.attribute(declaration.c_str()).value();
  const auto type = local(name);
  return (type == "Catalogue" || type == "Configuration" || type == "Assembly"
          || type == "Schedule" || type == "LinkBase")
         && ns.ends_with("/" + type);
}

// `value`, decoded from XML, as an XSMP literal.
std::string quote(std::string_view value, char delimiter = '"') {
  return delimiter + cppEscaped(value, delimiter) + delimiter;
}

// `value` as an XSMP name: a character other than a letter, a digit, `_`, `.`
// or one of `kept` becomes `_`, and a leading digit gets a `_` before it.
// Throws when `value` is empty.
std::string identifier(std::string value, std::string_view kept = {}) {
  for (auto& c : value)
    if (!std::isalnum(static_cast<unsigned char>(c)) && c != '_' && c != '.'
        && kept.find(c) == kept.npos)
      c = '_';
  if (value.empty())
    throw std::runtime_error("Missing SMP element name or reference");
  if (std::isdigit(static_cast<unsigned char>(value[0])))
    value.insert(value.begin(), '_');
  return value;
}

bool validQualifiedName(std::string_view value) {
  auto first = true;
  for (const auto c : value) {
    const auto letter = std::isalpha(static_cast<unsigned char>(c)) != 0;
    if (first) {
      if (!letter && c != '_') return false;
      first = false;
    } else if (c == '.')
      first = true;
    else if (!std::isalnum(static_cast<unsigned char>(c)) && c != '_')
      return false;
  }
  return !first;
}

// `header` and `body` in braces, one indent deeper.
std::string block(std::string header, std::string body) {
  auto result = std::move(header) + "\n{\n";
  std::istringstream lines(body);
  for (std::string line; std::getline(lines, line);)
    result += "    " + line + '\n';
  return result + "}\n";
}

// `body` as a doc comment, or nothing when it is empty.
std::string docComment(std::string body) {
  if (body.empty()) return {};
  // `*/` would end the comment.
  for (auto at = body.find("*/"); at != body.npos; at = body.find("*/", at + 3))
    body.replace(at, 2, "* /");
  std::string result = "/**\n";
  std::istringstream lines(body);
  for (std::string line; std::getline(lines, line);)
    result += " * " + line + '\n';
  return result + " */\n";
}

// The doc comment of an element: its description, `extraTags`, then one tag per
// attribute and child that XSMP has no syntax for. Empty when there is nothing
// to say.
std::string comment(pugi::xml_node node,
                    const std::vector<std::string>& extraTags = {}) {
  auto body = text(node, "Description");
  for (const auto& tag : extraTags) {
    if (!body.empty()) body += '\n';
    body += tag;
  }
  for (const auto* key :
       {"Id", "Uuid", "Title", "Date", "Creator", "Version", "Category", "Unit",
        "Namespace", "Location", "Platform", "Usage"}) {
    auto value = attr(node, key);
    if (value.empty()) continue;
    if (!body.empty()) body += '\n';
    body += "@" + lower(key) + " " + value;
  }
  if (boolean(node, "AllowMultiple")) body += "\n@allowMultiple";
  for (auto c : node.children()) {
    if (local(c.name()) == "Usage")
      body += "\n@usage " + decodeXmlText(c.text().get());
    if (local(c.name()) == "Platform")
      for (const auto* key : {"Name", "Type", "Namespace", "Location"}) {
        auto value = attr(c, key);
        if (value.empty()) continue;
        body += "\n@"
                + (std::string_view(key) == "Name" ? std::string("platform")
                                                   : lower(key))
                + " " + value;
      }
  }
  if (!boolean(node, "Multicast", true)) body += "\n@singlecast";
  return docComment(std::move(body));
}

// Maps the `Id` of each element under `node` to its qualified name, which
// leaves out the name of a catalogue.
void indexNames(pugi::xml_node node, std::string prefix,
                std::unordered_map<std::string, std::string>& names) {
  const auto tag = local(node.name());
  const auto name = attr(node, "Name");
  if (tag != "Catalogue" && !name.empty())
    prefix += (prefix.empty() ? "" : ".") + name;
  const auto id = attr(node, "Id");
  if (!id.empty() && !prefix.empty()) names[id] = prefix;
  for (auto c : node.children())
    if (c.type() == pugi::node_element) indexNames(c, prefix, names);
}

// From the `Id` of each element of the builtin libraries to its qualified name.
// An `Id` there can be opaque, as `_12_1_910029a_`. Read once, by the first
// link that leaves its document.
const std::unordered_map<std::string, std::string>& builtinNames(
    const cursiv::Documents* documents) {
  static const auto names = [documents] {
    std::unordered_map<std::string, std::string> result;
    const auto read = [&result](const cursiv::Document& document) {
      for (const auto* node : document.parseResult.nodes())
        if (cursiv::ast_cast<NamedElement>(node))
          if (auto id = docTag(*node, "id"))
            result.emplace(std::move(*id), qualifiedName(*node));
    };
    std::unique_ptr<XsmpSharedCoreServices> shared;
    for (const auto& source : builtinSources()) {
      if (const auto loaded =
              documents ? documents->getDocument(source.uri) : nullptr;
          loaded && loaded->parseResult.value) {
        read(*loaded);
        continue;
      }
      if (!shared) {
        shared = createXsmpSharedServices();
        auto language = cursiv::make_default_core_services(*shared, "xsmpcat");
        generated::Xsmpcat::install_generated_module(*language);
        shared->serviceRegistry->registerServices(std::move(language));
      }
      read(*shared->documentFactory->fromString(std::string(source.text),
                                                source.uri));
    }
    return result;
  }();
  return names;
}

// Renders the parts of an SMP XML document as XSMP text, and collects warnings.
// A link can lead it to read another document.
struct Reader {
  explicit Reader(std::filesystem::path input, const ImportOptions& options)
      : file(std::filesystem::absolute(std::move(input))),
        searchRoots(options.searchRoots),
        documents(options.documents),
        references(options.references) {
    if (searchRoots.empty()) searchRoots.push_back(file.parent_path());
  }

  std::filesystem::path file;
  // From `Id` to qualified name, for the input document.
  std::unordered_map<std::string, std::string> names;
  std::vector<std::string> warnings;
  // From qualified name to `Type` element, for the input document.
  std::unordered_map<std::string, pugi::xml_node> declarationTypes;
  std::vector<std::filesystem::path> searchRoots;
  const cursiv::Documents* documents;
  std::set<std::string>* references;
  // The other documents, read once each.
  std::map<std::filesystem::path, std::unique_ptr<pugi::xml_document>> external;
  // From `Id` to qualified name, for each of them.
  std::map<pugi::xml_node, std::unordered_map<std::string, std::string>>
      externalNames;
  // The files under the search roots by name, from one walk.
  std::optional<
      std::unordered_map<std::string, std::vector<std::filesystem::path>>>
      files;
  std::set<std::string> reportedWarnings;

  // Reports each distinct message once.
  void warn(std::string message) {
    if (reportedWarnings.insert(message).second)
      warnings.push_back(std::move(message));
  }

  const auto& filesUnderRoots() {
    if (files) return *files;
    files.emplace();
    std::error_code error;
    for (const auto& root : searchRoots)
      for (std::filesystem::recursive_directory_iterator it(
               root, std::filesystem::directory_options::skip_permission_denied,
               error),
           end;
           it != end; it.increment(error)) {
        if (error) {
          error.clear();
          continue;
        }
        if (it->is_directory(error)) {
          if (ignoredDirectory(it->path().filename().string()))
            it.disable_recursion_pending();
        } else if (it->is_regular_file(error))
          (*files)[it->path().filename().string()].push_back(
              std::filesystem::absolute(it->path()).lexically_normal());
      }
    return *files;
  }

  // The file that `href` points to: beside the input when it exists there, else
  // the only match under the search roots. Empty for a URL and for no match,
  // and for several matches, which also gives a warning.
  std::filesystem::path externalPath(std::string href, std::string_view label) {
    if (auto hash = href.find('#'); hash != href.npos) href.resize(hash);
    if (href.empty()) return {};
    if (href.starts_with("file:")) {
      const auto path = cursiv::file_uri_to_path(href);
      if (!path) return {};
      href = *path;
    } else if (href.find("://") != href.npos)
      return {};
    std::ranges::replace(href, '\\', '/');

    const std::filesystem::path relative(href);
    if (references) references->insert(relative.filename().string());
    const auto direct =
        (relative.is_absolute() ? relative : file.parent_path() / relative)
            .lexically_normal();
    std::error_code error;
    if (std::filesystem::is_regular_file(direct, error)) return direct;

    std::set<std::filesystem::path> matches;
    const auto& candidates = filesUnderRoots();
    if (const auto at = candidates.find(relative.filename().string());
        at != candidates.end())
      for (const auto& candidate : at->second)
        if (href.find('/') == href.npos
            || candidate.generic_string().ends_with(href))
          matches.insert(candidate);

    if (matches.size() == 1) return *matches.begin();
    if (matches.size() > 1) {
      auto message = "Multiple SMP files match external " + std::string(label)
                     + " href '" + href + "': ";
      auto first = true;
      for (const auto& match : matches) {
        message += (first ? "'" : ", '") + match.string() + "'";
        first = false;
      }
      warn(message + ". Not selecting one.");
    }
    return {};
  }

  // The root of the document that `href` points to. Null when there is none,
  // and when it is not an SMP document, which also gives a warning.
  pugi::xml_node externalDocument(const std::string& href,
                                  std::string_view label) {
    auto path = externalPath(href, label);
    if (path.empty()) return {};
    auto at = external.find(path);
    if (at == external.end()) {
      auto xml = std::make_unique<pugi::xml_document>();
      if (!xml->load_file(path.c_str())) return {};
      at = external.emplace(path, std::move(xml)).first;
    }
    const auto root = at->second->document_element();
    if (!supportedDocument(root)) {
      warn("External " + std::string(label) + " file '" + path.string() +
           "' is not a supported SMP import document; falling back to title or "
           "fragment text.");
      return {};
    }
    return root;
  }

  // How a warning calls this link.
  static std::string referenceLabel(pugi::xml_node link) {
    const auto name = local(link.name());
    if (name == "Task") return "task reference";
    if (name == "Configuration") return "configuration include";
    if (name == "ItemType") return "array item type";
    if (name == "Type") {
      return lower(local(link.parent().name())) + " type";
    }
    return "reference";
  }

  // `name` as an identifier, without the namespaces that enclose `node`.
  static std::string localName(pugi::xml_node node, std::string name) {
    std::string currentNamespace;
    for (auto owner = node.parent(); owner; owner = owner.parent())
      if (local(owner.name()) == "Namespace")
        currentNamespace =
            attr(owner, "Name")
            + (currentNamespace.empty() ? "" : "." + currentNamespace);
    if (!currentNamespace.empty() && name.starts_with(currentNamespace + "."))
      name.erase(0, currentNamespace.size() + 1);
    return identifier(name);
  }

  // The XSMP text of a link. Its `#fragment` is looked up in this document or
  // in the other one. Failing that, the title or the fragment itself gives the
  // name. `__missing__` and a warning when the link has nothing to give.
  std::string reference(pugi::xml_node link) {
    if (!link) {
      warn("Missing reference; using placeholder.");
      return "__missing__";
    }
    auto href = attr(link, "href");
    auto title = attr(link, "title");
    const auto hash = href.find('#');
    auto fragment = hash == href.npos ? "" : href.substr(hash + 1);
    if (!fragment.empty()) {
      if (hash == 0) {
        if (auto at = names.find(fragment); at != names.end()) {
          auto result = at->second;
          auto root = link;
          while (root.parent() && root.parent().type() != pugi::node_document)
            root = root.parent();
          // Names in a schedule start with the name of the schedule, which XSMP
          // leaves out.
          const auto prefix = attr(root, "Name") + ".";
          if (local(root.name()) == "Schedule" && result.starts_with(prefix))
            result.erase(0, prefix.size());
          return localName(link, result);
        }
      } else {
        const auto label = referenceLabel(link);
        if (const auto root = externalDocument(href, label)) {
          const auto [names, fresh] = externalNames.try_emplace(root);
          if (fresh) indexNames(root, {}, names->second);
          if (auto at = names->second.find(fragment); at != names->second.end())
            return identifier(at->second);
          warn("Could not resolve external " + label + " id '" + fragment
               + "' from '" + href
               + "'; falling back to title or fragment text.");
        }
        const auto& builtins = builtinNames(documents);
        if (const auto at = builtins.find(fragment); at != builtins.end())
          return at->second;
      }
      const auto qualified = std::ranges::all_of(fragment, [](unsigned char c) {
        return std::isalnum(c) || c == '_' || c == '.';
      });
      if (!qualified && !title.empty()) return identifier(title);
      return identifier(fragment);
    }
    if (!title.empty()) return identifier(title);
    if (!href.empty()) return documentReference(href);
    warn("Missing " + referenceLabel(link) + " target; using placeholder.");
    return "__missing__";
  }

  std::string reference(pugi::xml_node node, std::string_view name) {
    return reference(child(node, name));
  }

  // The name of the document that `value` points to, or else its file stem with
  // a warning.
  std::string documentReference(std::string value,
                                std::string_view label = "document reference",
                                std::string_view expectedKind = {}) {
    if (auto fragment = value.find('#'); fragment != value.npos)
      value.resize(fragment);
    if (value.empty()) {
      warn("Missing " + std::string(label) + "; using placeholder.");
      return "__missing__";
    }
    if (const auto root = externalDocument(value, label)) {
      const auto actual = local(root.name());
      if (expectedKind.empty() || actual == expectedKind)
        return identifier(attr(root, "Name"));
      warn("External " + std::string(label) + " file '"
           + externalPath(value, label).string() + "' is a " + lower(actual)
           + " document, expected " + lower(std::string(expectedKind))
           + "; falling back to title or fragment text.");
    }
    const auto fallback =
        identifier(std::filesystem::path(value).stem().string());
    warn("Could not resolve " + std::string(label) + " '" + value + "'; using '"
         + fallback + "'.");
    return fallback;
  }

  // The `Type` element that a link points to, here or in another document. Null
  // when it cannot be found.
  pugi::xml_node referencedType(pugi::xml_node link) {
    if (!link) return {};
    auto name = reference(link);
    auto at = declarationTypes.find(name);
    if (at != declarationTypes.end()) return at->second;
    const auto href = attr(link, "href");
    const auto hash = href.find('#');
    const auto fragment = hash == href.npos ? "" : href.substr(hash + 1);
    if (const auto full = names.find(fragment); full != names.end())
      if (const auto type = declarationTypes.find(full->second);
          type != declarationTypes.end())
        return type->second;
    if (!href.empty() && !href.starts_with('#'))
      if (auto root = externalDocument(href, referenceLabel(link))) {
        const auto find = [&](const auto& self,
                              pugi::xml_node node) -> pugi::xml_node {
          if (local(node.name()) == "Type" && attr(node, "Id") == fragment)
            return node;
          for (auto c : node.children())
            if (auto match = self(self, c)) return match;
          return {};
        };
        return find(find, root);
      }
    return {};
  }

  // A value as XSMP text. `catalogue` selects the expression syntax of a
  // catalogue; deployment documents use literal suffixes instead. `impliedType`
  // is the kind of an item of a simple array, which has no `xsi:type`.
  // `expected`, the declared type, tells a `nullptr` from a string and names
  // enumeration literals.
  std::string typedValue(pugi::xml_node node, bool catalogue = false,
                         std::string impliedType = {},
                         pugi::xml_node expected = {}) {
    if (!node) {
      warn("Missing SMP value; defaulting to 0.");
      return catalogue ? "nullptr" : "0";
    }

    const auto type = kind(node).empty() ? impliedType : kind(node);
    const auto value = attr(node, "Value", "0");
    if (type == "Value" || type.empty()) return catalogue ? "nullptr" : "0";
    if (type == "BoolValue") return boolean(node, "Value") ? "true" : "false";

    if (type == "String8Value") {
      if (catalogue && attr(node, "Value") == "nullptr" && expected) {
        const auto t = kind(expected);
        if (t == "NativeType" || t == "ValueReference" || t == "Class"
            || t == "Exception" || t == "Interface" || t == "Model"
            || t == "Service")
          return "nullptr";
      }
      return quote(attr(node, "Value"));
    }

    if (type == "Char8Value") return quote(attr(node, "Value"), '\'');
    if (type == "DurationValue" || type == "DateTimeValue")
      return quote(attr(node, "Value"))
             + (catalogue                 ? ""
                : type == "DurationValue" ? "d"
                                          : "dt");

    if (type == "EnumerationValue") {
      auto literal = attr(node, "Literal");
      if (catalogue && expected && kind(expected) == "Enumeration") {
        if (literal.empty())
          for (auto c : expected.children())
            if (local(c.name()) == "Literal" && attr(c, "Value") == value) {
              literal = attr(c, "Name");
              break;
            }
        if (!literal.empty()) {
          if (literal.find('.') != literal.npos) return identifier(literal);
          auto at = names.find(attr(expected, "Id"));
          return localName(node, at == names.end() ? attr(expected, "Name")
                                                   : at->second)
                 + "." + identifier(literal);
        }
      }
      // A deployment document has no scope that holds a bare literal.
      if (catalogue && !literal.empty()) return identifier(literal);
      return value + (catalogue ? "" : "lit");
    }

    // Outside a catalogue, a number carries its type as a suffix: `1i32`.
    const std::map<std::string, std::string> suffixes{
        {"Int8Value", "i8"},    {"Int16Value", "i16"},  {"Int32Value", "i32"},
        {"Int64Value", "i64"},  {"UInt8Value", "u8"},   {"UInt16Value", "u16"},
        {"UInt32Value", "u32"}, {"UInt64Value", "u64"}, {"Float32Value", "f32"},
        {"Float64Value", "f64"}};
    if (auto at = suffixes.find(type); at != suffixes.end()) {
      auto numeric = value;
      // `INF` and `NaN` have no XSMP form.
      if (numeric.find_first_of("0123456789") == numeric.npos) {
        warn("The value '" + numeric
             + "' is not representable in XSMP and was replaced by 0.");
        numeric = "0";
      }
      // A float needs a point or an exponent to read as one.
      if (type.starts_with("Float")
          && numeric.find_first_of(".eE") == numeric.npos)
        numeric += ".";
      if (catalogue) {
        if (type.starts_with("UInt"))
          return numeric + (type == "UInt64Value" ? "UL" : "U");
        if (type == "Float32Value") return numeric + "f";
        return numeric;
      }
      return numeric + at->second;
    }

    if (type == "StructureValue" || type == "ArrayValue"
        || type.ends_with("ArrayValue")) {
      const auto structure = type == "StructureValue";
      if (!structure && catalogue) {
        const auto start = field(node, "StartIndex");
        if (!start.empty() && start != "0")
          warn("Catalogue array default value StartIndex='" + start
               + "' is not representable in XSMP and was discarded.");
      }
      std::string result = structure || catalogue ? "{" : "[";
      if (!structure && !catalogue) {
        auto start = field(node, "StartIndex");
        if (!start.empty() && start != "0") result += start + ": ";
      }
      auto first = true;
      std::size_t positionalIndex = 0;
      for (auto c : node.children())
        if (local(c.name()) == (structure ? "FieldValue" : "ItemValue")) {
          if (!first) result += ", ";
          first = false;
          auto name = attr(c, "Field");
          if (structure && !name.empty())
            result += (catalogue ? "." : "") + identifier(name) + " = ";
          // The declared type of this item: a field is found by name or by
          // position.
          pugi::xml_node expectedItem;
          if (expected && kind(expected) == "Array")
            expectedItem = referencedType(child(expected, "ItemType"));
          else if (expected && structure) {
            std::size_t fieldIndex = 0;
            for (auto f : expected.children())
              if (local(f.name()) == "Field") {
                if ((!name.empty() && attr(f, "Name") == name)
                    || (name.empty() && fieldIndex == positionalIndex)) {
                  expectedItem = referencedType(child(f, "Type"));
                  break;
                }
                ++fieldIndex;
              }
            if (name.empty()) ++positionalIndex;
          }
          // An item of `Int32ArrayValue` is an `Int32Value`, and so on.
          result += typedValue(c, catalogue,
                               !structure && type != "ArrayValue"
                                   ? type.substr(0, type.size() - 10) + "Value"
                                   : "",
                               expectedItem);
        }
      return result + (structure || catalogue ? "}" : "]");
    }
    warn(std::string(catalogue ? "Unsupported catalogue value '"
                               : "Unsupported imported SMP value '")
         + type + "'.");
    return catalogue ? "nullptr" : "0";
  }

  // A `<...>` list of template parameters or arguments, empty when there is
  // none.
  std::string parameters(pugi::xml_node node,
                         std::string_view element = "Parameter") {
    std::string result;
    for (auto p : node.children())
      if (local(p.name()) == element) {
        if (!result.empty()) result += ", ";
        const auto type = kind(p);
        result += identifier(attr(p, "Name"));
        if (type != "Int32Argument" && type != "StringArgument") {
          warn("Unsupported template "
               + std::string(element == "Argument" ? "argument" : "parameter")
               + " type '" + attr(p, "type", type) + "'; defaulting to "
               + (element == "Argument" ? "empty string." : "string."));
          result += element == "Argument" ? " = \"\"" : ": string";
          continue;
        }
        if (element == "Argument" && !p.attribute("Value")) {
          const auto integer = type == "Int32Argument";
          warn("Missing " + std::string(integer ? "int32" : "string")
               + " template argument value for '" + attr(p, "Name")
               + "'; defaulting to " + (integer ? "0." : "empty string."));
          result += integer ? " = 0" : " = \"\"";
          continue;
        }
        if (!p.attribute("Value"))
          result += type == "Int32Argument" ? ": int32" : ": string";
        else
          result += " = "
                    + (type == "Int32Argument" ? attr(p, "Value")
                                               : quote(attr(p, "Value")));
      }
    return result.empty() ? "" : "<" + result + ">";
  }

  std::string callParameters(pugi::xml_node node) {
    std::string result;
    for (auto p : node.children())
      if (local(p.name()) == "Parameter") {
        if (!result.empty()) result += ", ";
        result += identifier(attr(p, "Parameter", "__parameter__")) + " = "
                  + typedValue(child(p, "Value"));
      }
    return result;
  }

  // A link statement. An interface link joins `OwnerPath` and `Reference` back
  // into its source path.
  std::string link(pugi::xml_node node) {
    auto source = text(node, "OwnerPath");
    const auto target = text(node, "ClientPath", "__client__");
    const auto type = kind(node);
    if (type == "InterfaceLink") {
      const auto trim = [](std::string value) {
        const auto first = value.find_first_not_of(" \t\r\n");
        if (first == value.npos) return std::string{};
        return value.substr(first,
                            value.find_last_not_of(" \t\r\n") - first + 1);
      };
      source = trim(std::move(source));
      const auto ref = trim(text(node, "Reference"));
      if (!ref.empty())
        source = source.empty() || source == "." ? ref
                 : source == "/"                 ? source + ref
                                                 : source + "." + ref;
      else if (source.empty())
        source = ".";
      const auto back = text(node, "BackReference");
      return "interface link " + source + " -> " + target
             + (back.empty() ? "" : ":" + identifier(back)) + '\n';
    }

    if (type != "FieldLink" && type != "EventLink")
      warn("Unsupported link type '" + attr(node, "type", "LinkBase:Link")
           + "'; importing as field link.");
    if (source.empty()) source = "__owner__";
    return (type == "EventLink" ? "event link " : "field link ") + source
           + " -> " + target + '\n';
  }

  // The content of a configuration. `assembly` adds invocations and event
  // handlers.
  std::string configBody(pugi::xml_node node, bool assembly) {
    std::string body;
    for (auto c : node.children()) {
      const auto tag = local(c.name());
      if (tag == "FieldValue")
        body += attr(c, "Field") + " = " + typedValue(c) + '\n';
      else if (tag == "Include") {
        body += "include " + reference(c, "Configuration");
        const auto path = attr(c, "Path");
        if (!path.empty()) body += " at " + path;
        body += '\n';
      } else if (tag == "Component")
        body += block(attr(c, "Path", "."), configBody(c, false));
      else if (assembly && tag == "Invocation") {
        const auto type = kind(c);
        if (type == "OperationCall")
          body += "call " + identifier(attr(c, "Operation")) + "("
                  + callParameters(c) + ")\n";
        else if (type == "PropertyValue")
          body += "property " + identifier(attr(c, "Property")) + " = "
                  + typedValue(child(c, "Value")) + '\n';
        else {
          warn("Unsupported assembly invocation type '"
               + attr(c, "type", "Assembly:Invocation") + "'.");
          body += "/* unsupported invocation */\n";
        }
      } else if (assembly && tag == "GlobalEventHandler")
        body += "subscribe "
                + identifier(attr(c, "EntryPointName", "__entryPoint__"))
                + " -> " + quote(attr(c, "GlobalEventName")) + '\n';
    }
    return body;
  }

  std::string assemblyConfig(pugi::xml_node node) {
    return block("configure " + attr(node, "InstancePath", "."),
                 configBody(node, true));
  }

  // A model instance. With `sub`, it is added to its container:
  // `container += name`.
  std::string model(pugi::xml_node node, bool sub = false) {
    auto signature =
        sub ? identifier(attr(node, "Container", "__container__")) + " += "
            : "";
    signature += identifier(attr(node, "Name", "__model__"), "{}");

    auto implementation = attr(node, "Implementation");
    if (implementation.empty()) {
      warn(
          "Missing model implementation; using placeholder string "
          "implementation.");
      signature += ": \"__missing__\"";
    }
    // Written as an XSMP name when it can be, and as a string otherwise.
    if (!implementation.empty()) {
      const auto original = implementation;
      for (auto pos = implementation.find("::"); pos != implementation.npos;
           pos = implementation.find("::"))
        implementation.replace(pos, 2, ".");
      signature += ": "
                   + (validQualifiedName(implementation) ? implementation
                                                         : quote(original));
    }

    auto body = configBody(node, true);
    for (auto c : node.children()) {
      const auto tag = local(c.name());
      if (tag == "Model")
        body += model(c, true);
      else if (tag == "Link")
        body += link(c);
      else if (tag == "Assembly") {
        auto line = identifier(attr(c, "Container", "__container__")) + " += "
                    + identifier(attr(c, "Name", "__assembly__"), "{}") + ": "
                    + documentReference(field(c, "Assembly"),
                                        "assembly reference", "Assembly");
        line += parameters(c, "Argument");
        if (auto config = field(c, "Configuration"); !config.empty())
          line += " using config "
                  + documentReference(config, "assembly configuration",
                                      "Configuration");
        if (auto links = field(c, "LinkBase"); !links.empty())
          line += " using link "
                  + documentReference(links, "assembly link base", "LinkBase");
        std::string overrides;
        for (auto override : c.children())
          if (local(override.name()) == "ModelConfiguration")
            overrides += assemblyConfig(override);
        body += comment(c)
                + (overrides.empty() ? line + '\n' : block(line, overrides));
      }
    }
    return comment(node) + block(signature, body);
  }

  std::string componentLinks(pugi::xml_node node) {
    std::string body;
    for (auto c : node.children()) {
      if (local(c.name()) == "Link")
        body += link(c);
      else if (local(c.name()) == "Component")
        body += componentLinks(c);
    }
    return comment(node) + block(attr(node, "Path", "."), body);
  }

  // A statement of a task.
  std::string scheduleActivity(pugi::xml_node node) {
    const auto type = kind(node);
    if (type == "CallOperation")
      return "call " + text(node, "OperationPath", "__operation__") + "("
             + callParameters(node) + ")\n";
    if (type == "SetProperty")
      return "property " + text(node, "PropertyPath", "__property__") + " = "
             + typedValue(child(node, "Value")) + '\n';
    if (type == "Transfer")
      return "transfer " + text(node, "OutputFieldPath", "__output__") + " -> "
             + text(node, "InputFieldPath", "__input__") + '\n';
    if (type == "Trigger")
      return "trig " + text(node, "EntryPoint", "__entryPoint__") + '\n';

    if (type == "EmitGlobalEvent") {
      // Accepted as an attribute or as a child element, which wins.
      auto synchronous = boolean(node, "synchronous", true);
      if (const auto value = child(node, "synchronous")) {
        const std::string_view text = value.text().get();
        if (text == "true" || text == "1")
          synchronous = true;
        else if (text == "false" || text == "0")
          synchronous = false;
        else
          throw std::runtime_error("Invalid XML Schema boolean value '"
                                   + std::string(text)
                                   + "' for element 'synchronous'.");
      }
      return (synchronous ? "emit " : "async emit ")
             + quote(text(node, "EventName")) + '\n';
    }

    if (type == "ExecuteTask") {
      auto result =
          "execute " + reference(node, "Task") + parameters(node, "Argument");
      if (auto root = attr(node, "Root"); !root.empty())
        result += " at " + root;
      return result + '\n';
    }

    warn("Unsupported schedule activity type '"
         + attr(node, "type", "Schedule:Activity") + "'.");
    return "/* unsupported activity */\n";
  }

  // An event statement.
  std::string scheduleEvent(pugi::xml_node node) {
    auto result = "event " + reference(node, "Task");
    const auto type = kind(node);
    if (type == "GlobalEventTriggeredEvent") {
      result += " on " + quote(attr(node, "StartEvent"));
      if (auto stop = attr(node, "StopEvent"); !stop.empty())
        result += " until " + quote(stop);
      auto time = attr(node, "TimeKind");
      if (!time.empty()) {
        if (time.ends_with("Time")) time.resize(time.size() - 4);
        result += " using " + lower(time);
      }
      if (auto delay = attr(node, "Delay"); !delay.empty())
        result += " delay " + quote(delay);
    } else {
      const std::map<std::string, std::string> kinds{
          {"MissionEvent", "mission"},
          {"EpochEvent", "epoch"},
          {"SimulationEvent", "simulation"},
          {"ZuluEvent", "zulu"}};
      auto at = kinds.find(type);
      if (at == kinds.end()) {
        warn("Unsupported schedule event type '"
             + attr(node, "type", "Schedule:Event") + "'.");
        auto task = reference(node, "Task");
        const auto dot = task.rfind('.');
        if (dot != task.npos) task.erase(0, dot + 1);
        return "/* unsupported event for " + task + " */\n";
      }
      // `MissionEvent` holds its time in `MissionTime`, and so on.
      result += " " + at->second + " "
                + quote(attr(node, type.substr(0, type.size() - 5) + "Time"));
    }

    auto cycle = attr(node, "CycleTime");
    auto repeat = attr(node, "RepeatCount");
    if ((!cycle.empty() && cycle != "PT0S") || !repeat.empty()) {
      result += " cycle " + quote(cycle.empty() ? "PT0S" : cycle);
      if (!repeat.empty()) result += " repeat " + repeat;
    }
    return result + '\n';
  }

  std::string visibility(pugi::xml_node node) {
    auto v = attr(node, "Visibility");
    return v.empty() ? "" : v + " ";
  }

  // The `@Name(value)` lines of the attributes of an element. A value equal to
  // the default of the attribute type is left out.
  std::string applied(pugi::xml_node node) {
    std::string result;
    for (auto c : node.children())
      if (local(c.name()) == "Metadata" && kind(c) == "Attribute") {
        const auto attributeName = reference(c, "Type");
        result += "@" + attributeName;
        if (auto value = child(c, "Value")) {
          const auto attributeType = referencedType(child(c, "Type"));
          const auto underlying = referencedType(child(attributeType, "Type"));
          auto rendered = typedValue(value, true, "", underlying);
          // ECSS builtins are represented by standard URIs, not files. The
          // View attribute's enumeration is nevertheless unambiguous.
          if (!underlying && attributeName == "Attributes.View"
              && !attr(value, "Literal").empty())
            rendered = "ViewKind." + identifier(attr(value, "Literal"));
          const auto defaultValue = child(attributeType, "Default");
          if (!defaultValue || !underlying
              || typedValue(defaultValue, true, "", underlying) != rendered)
            result += "(" + rendered + ")";
        }
        result += '\n';
      } else if (local(c.name()) == "Metadata") {
        warn("Unsupported metadata entry '" + attr(c, "type", "unknown")
             + "' was ignored.");
      }
    return result;
  }

  // ` in min ... max`, with `*` for a missing bound. Only a Float can exclude a
  // bound.
  std::string range(pugi::xml_node node, bool floating) {
    const auto minimum = attr(node, "Minimum");
    const auto maximum = attr(node, "Maximum");
    if (minimum.empty() && maximum.empty()) return {};
    const auto minInclusive = boolean(node, "MinInclusive", true);
    const auto maxInclusive = boolean(node, "MaxInclusive", true);
    const auto op = floating ? (minInclusive ? (maxInclusive ? "..." : "..<")
                                             : (maxInclusive ? "<.." : "<.<"))
                             : "...";
    return " in " + (minimum.empty() ? "*" : minimum) + " " + op + " "
           + (maximum.empty() ? "*" : maximum);
  }

  std::string extends(pugi::xml_node node, std::string_view key) {
    return child(node, key) ? " extends " + reference(node, key) : "";
  }

  std::string defaultValue(pugi::xml_node node, std::string_view key) {
    const auto value = child(node, key);
    if (!value) return "";
    if (kind(value).empty()) return cppDefault(value.text().get());
    return " = "
           + typedValue(value, true, "", referencedType(child(node, "Type")));
  }

  // A default without a type: in ECSS SMP 2025 the default of a Parameter is
  // the text of a C++ expression. What XSMP cannot read as an expression is
  // dropped.
  std::string cppDefault(std::string_view written) {
    const auto first = written.find_first_not_of(" \t\r\n");
    if (first == written.npos) return "";
    written =
        written.substr(first, written.find_last_not_of(" \t\r\n") - first + 1);
    auto expression = std::string(written);
    // A C++ name, but never inside a string or a character.
    if (written.find_first_of("\"'") == written.npos) {
      if (expression.starts_with("::")) expression.erase(0, 2);
      for (auto at = expression.find("::"); at != expression.npos;
           at = expression.find("::", at))
        expression.replace(at, 2, ".");
    }
    const auto parser = generated::Xsmpcat::create_parser();
    const auto probe =
        "catalogue c namespace n { interface i { def void f(in t p = "
        + expression + ") } }";
    const auto syntax = parser->parse(probe);
    if (syntax.accepted && syntax.value && syntax.diagnostics.empty())
      return " = " + expression;
    warn("Parameter default '" + std::string(written)
         + "' is not representable in XSMP and was discarded.");
    return "";
  }

  std::string commaReferences(pugi::xml_node node, std::string_view key) {
    std::string result;
    for (auto c : node.children())
      if (local(c.name()) == key) {
        if (!result.empty()) result += ", ";
        result += reference(c);
      }
    return result;
  }

  std::string multiplicity(pugi::xml_node node) {
    const auto lower = attr(node, "Lower", "1");
    const auto upper = attr(node, "Upper", "1");
    if (lower == "1" && upper == "1") return "";
    if (lower == "0" && upper == "1") return "?";
    if (upper == "-1")
      return lower == "0" ? "*" : lower == "1" ? "+" : "[" + lower + " ... *]";
    return "[" + lower + " ... " + upper + "]";
  }

  // A member of a type.
  std::string catalogueMember(pugi::xml_node node) {
    const auto tag = local(node.name());
    const auto name = identifier(attr(node, "Name"));
    const auto header = comment(node) + applied(node);
    const auto ownerKind = kind(node.parent());
    // A member of an Interface or a Structure is written without visibility.
    const auto vis = ownerKind == "Interface" || ownerKind == "Structure"
                         ? std::string{}
                         : visibility(node);

    if (tag == "Constant")
      return header + vis + "constant " + reference(node, "Type") + " " + name
             + defaultValue(node, "Value") + '\n';

    if (tag == "Field") {
      auto flags = vis;
      if (boolean(node, "Input")) flags += "input ";
      if (boolean(node, "Output")) flags += "output ";
      if (!boolean(node, "State", true)) flags += "transient ";
      return header + flags + "field " + reference(node, "Type") + " " + name
             + defaultValue(node, "Default") + '\n';
    }

    if (tag == "Property") {
      auto access = attr(node, "Access");
      auto result = header + vis + (access.empty() ? "" : access + " ")
                    + "property " + reference(node, "Type") + " " + name;
      if (auto raised = commaReferences(node, "GetRaises"); !raised.empty())
        result += " get throws " + raised;
      if (auto raised = commaReferences(node, "SetRaises"); !raised.empty())
        result += " set throws " + raised;
      if (child(node, "AttachedField"))
        result += " -> " + reference(node, "AttachedField");
      return result + '\n';
    }

    if (tag == "Association")
      return header + vis + "association " + reference(node, "Type") + " "
             + name + '\n';

    if (tag == "Container")
      return header + "container " + reference(node, "Type")
             + multiplicity(node) + " " + name
             + (child(node, "DefaultComponent")
                    ? " = " + reference(node, "DefaultComponent")
                    : "")
             + '\n';

    // The link is `Type` in ECSS SMP 2025, and `Interface` in 2020.
    if (tag == "Reference")
      return header + "reference "
             + reference(node, child(node, "Type") ? "Type" : "Interface")
             + multiplicity(node) + " " + name + '\n';

    if (tag == "EventSink" || tag == "EventSource")
      return header + (tag == "EventSink" ? "eventsink " : "eventsource ")
             + reference(node, "Type") + " " + name + '\n';

    if (tag == "EntryPoint") {
      std::string body;
      for (auto c : node.children())
        if (local(c.name()) == "Input" || local(c.name()) == "Output")
          body += (local(c.name()) == "Input" ? "in " : "out ") + reference(c)
                  + '\n';
      return header
             + (body.empty() ? "entrypoint " + name + '\n'
                             : block("entrypoint " + name, body));
    }

    if (tag == "Operation") {
      std::string returned = "void", parametersText;
      std::vector<std::string> parameterDescriptions;
      for (auto p : node.children())
        if (local(p.name()) == "Parameter") {
          auto parameter = reference(p, "Type");
          const auto attributes = applied(p);
          const auto direction = attr(p, "Direction");
          // XSMP keeps the description of a parameter in the doc comment of the
          // Operation.
          const auto description = text(p, "Description");
          if (!description.empty())
            parameterDescriptions.push_back(direction == "return"
                                                ? "@return " + description
                                                : "@param " + attr(p, "Name")
                                                      + " " + description);
          if (direction == "return") {
            const auto returnName = attr(p, "Name");
            returned = attributes + parameter
                       + (returnName.empty() || returnName == "return"
                              ? ""
                              : " " + identifier(returnName));
          } else {
            if (!parametersText.empty()) parametersText += ", ";
            parametersText += attributes
                              + (direction.empty() ? "" : direction + " ")
                              + parameter + " " + identifier(attr(p, "Name"))
                              + defaultValue(p, "Default");
          }
        }
      auto result = comment(node, parameterDescriptions) + applied(node) + vis
                    + "def " + returned + " " + name + "(" + parametersText
                    + ")";
      if (auto raised = commaReferences(node, "RaisedException");
          !raised.empty())
        result += " throws " + raised;
      return result + '\n';
    }

    throw std::runtime_error("Unsupported SMP member '" + tag + "'");
  }

  // A type of a namespace.
  std::string catalogueType(pugi::xml_node node) {
    const auto type = kind(node);
    const auto name = identifier(attr(node, "Name"));
    const auto header = comment(node) + applied(node);
    const auto vis = visibility(node);

    if (type == "PrimitiveType" || type == "NativeType")
      return header + vis + (type == "PrimitiveType" ? "primitive " : "native ")
             + name + '\n';

    if (type == "AttributeType")
      return header + vis + "attribute " + reference(node, "Type") + " " + name
             + defaultValue(node, "Default") + '\n';

    if (type == "Array")
      return header + vis + "array " + name + " = "
             + reference(node, "ItemType") + "[" + attr(node, "Size", "0")
             + "]\n";

    if (type == "ValueReference")
      return header + vis + "using " + name + " = " + reference(node, "Type")
             + "*\n";

    if (type == "String")
      return header + vis + "string " + name + "[" + attr(node, "Length", "0")
             + "]\n";

    if (type == "Integer" || type == "Float")
      return header + vis + (type == "Integer" ? "integer " : "float ") + name
             + extends(node, "PrimitiveType") + range(node, type == "Float")
             + '\n';

    if (type == "EventType")
      return header + vis + "event " + name + extends(node, "EventArgs") + '\n';

    if (type == "Enumeration") {
      std::string body;
      for (auto c : node.children())
        if (local(c.name()) == "Literal") {
          if (!body.empty()) body += ",\n";
          body += comment(c) + identifier(attr(c, "Name")) + " = "
                  + attr(c, "Value", "0");
        }
      return header + block(vis + "enum " + name, body);
    }

    const std::map<std::string, std::string> keywords{
        {"Structure", "struct"},    {"Class", "class"},
        {"Exception", "exception"}, {"Interface", "interface"},
        {"Model", "model"},         {"Service", "service"}};
    auto at = keywords.find(type);
    if (at == keywords.end()) {
      warn("Unsupported catalogue type '" + attr(node, "type") + "'.");
      return "/* Unsupported type " + attr(node, "type") + " " + name + " */\n";
    }

    auto signature = vis + (boolean(node, "Abstract") ? "abstract " : "")
                     + at->second + " " + name;
    auto base = commaReferences(node, "Base");
    if (!base.empty()) signature += " extends " + base;
    auto interfaces = commaReferences(node, "Interface");
    if (!interfaces.empty()) signature += " implements " + interfaces;

    std::string body;
    for (auto c : node.children()) {
      const auto tag = local(c.name());
      if (tag == "Constant" || tag == "Field" || tag == "Property"
          || tag == "Operation" || tag == "Association" || tag == "Container"
          || tag == "Reference" || tag == "EntryPoint" || tag == "EventSink"
          || tag == "EventSource")
        body += catalogueMember(c);
      else if (tag == "Realization")
        warn("Realization '" + attr(c, "Name") + "' of '" + name
             + "' is not representable in XSMP and was discarded.");
    }
    return header + block(signature, body);
  }

  std::string catalogueNamespace(pugi::xml_node node) {
    std::string body;
    for (auto c : node.children()) {
      if (local(c.name()) == "Namespace")
        body += catalogueNamespace(c);
      else if (local(c.name()) == "Type")
        body += catalogueType(c);
    }
    return comment(node) + applied(node)
           + block("namespace " + identifier(attr(node, "Name")), body);
  }
};

// The comments before the root as lines of XSMP text: the export writes the
// copyright notice of a document there.
std::string copyrightNotice(const pugi::xml_document& xml) {
  std::string notice;
  for (const auto node : xml.children()) {
    if (node.type() == pugi::node_element) break;
    if (node.type() != pugi::node_comment) continue;
    const auto decoded = decodeXmlText(node.value());
    std::string_view text = decoded;
    while (!text.empty()) {
      const auto end = text.find('\n');
      auto line = text.substr(0, end);
      while (!line.empty()
             && std::isspace(static_cast<unsigned char>(line.back())))
        line.remove_suffix(1);
      notice += "// " + std::string(line) + "\n";
      if (end == text.npos) break;
      text.remove_prefix(end + 1);
    }
  }
  return notice.empty() ? notice : notice + "\n";
}

void removeComments(pugi::xml_node node) {
  for (auto child = node.first_child(); child;) {
    const auto next = child.next_sibling();
    if (child.type() == pugi::node_comment)
      node.remove_child(child);
    else
      removeComments(child);
    child = next;
  }
}

}  // namespace

ImportResult renderImportedDocument(const std::filesystem::path& input,
                                    const ImportOptions& options) {
  pugi::xml_document xml;
  // A text of spaces alone is a value too: `<EventName> </EventName>`.
  const auto parsed = xml.load_file(
      input.c_str(), pugi::parse_default | pugi::parse_ws_pcdata_single
                         | pugi::parse_comments);
  if (!parsed)
    throw std::runtime_error("Cannot parse SMP XML '" + input.string()
                             + "': " + parsed.description());
  const auto root = xml.document_element();
  const auto notice = copyrightNotice(xml);
  removeComments(root);
  for (const auto node : xml.children())
    if (node.type() == pugi::node_element && node != root)
      throw std::runtime_error("Cannot parse SMP XML '" + input.string()
                               + "': multiple root elements");
  const auto rootName = local(root.name());
  if (!supportedDocument(root))
    throw std::runtime_error("Unsupported SMP XML root '"
                             + std::string(root.name()) + "'");

  Reader reader(input, options);
  // Every name is indexed first, as a link can point forward.
  indexNames(root, {}, reader.names);
  const auto indexTypes = [&](const auto& self, pugi::xml_node node) -> void {
    if (local(node.name()) == "Type" && !kind(node).empty()) {
      auto at = reader.names.find(attr(node, "Id"));
      if (at != reader.names.end())
        reader.declarationTypes.emplace(at->second, node);
    }
    for (auto c : node.children()) self(self, c);
  };
  indexTypes(indexTypes, root);

  ImportResult result;
  std::unique_ptr<cursiv::Parser> parser;
  std::string extension;
  // The schemas require a `Name`; a placeholder keeps a malformed document
  // importable.
  const auto name =
      identifier(attr(root, "Name",
                      rootName == "Assembly"    ? "__assembly__"
                      : rootName == "Schedule"  ? "__schedule__"
                      : rootName == "LinkBase"  ? "__link__"
                      : rootName == "Catalogue" ? "__catalogue__"
                                                : "__configuration__"));

  // A keyword, the template parameters if there are any, and the name.
  const auto withParameters = [&](std::string_view keyword) {
    const auto parameters = reader.parameters(root);
    return std::string(keyword) + " "
           + (parameters.empty() ? parameters : parameters + " ") + name;
  };

  std::string body;
  if (rootName == "Catalogue") {
    result.kind = "catalogue";
    extension = ".xsmpcat";
    parser = generated::Xsmpcat::create_parser();
    body = comment(root) + reader.applied(root) + "catalogue " + name + "\n\n";
    for (auto c : root.children())
      if (local(c.name()) == "Namespace") body += reader.catalogueNamespace(c);
  } else if (rootName == "Configuration") {
    result.kind = "configuration";
    extension = ".xsmpcfg";
    parser = generated::Xsmpcfg::create_parser();
    body = comment(root) + "configuration " + name + '\n'
           + reader.configBody(root, false);
  } else if (rootName == "LinkBase") {
    result.kind = "linkbase";
    extension = ".xsmplnk";
    parser = generated::Xsmplnk::create_parser();
    body = comment(root) + "link " + name + '\n';
    for (auto c : root.children())
      if (local(c.name()) == "Component") body += reader.componentLinks(c);
  } else if (rootName == "Assembly") {
    result.kind = "assembly";
    extension = ".xsmpasb";
    parser = generated::Xsmpasb::create_parser();
    body = comment(root) + withParameters("assembly") + '\n';
    for (auto c : root.children())
      if (local(c.name()) == "ComponentConfiguration")
        body += reader.assemblyConfig(c);
    if (!child(root, "Model"))
      throw std::runtime_error(
          "Malformed SMP Assembly XML: missing root Model element");
    body += reader.model(child(root, "Model"));
  } else if (rootName == "Schedule") {
    result.kind = "schedule";
    extension = ".xsmpsed";
    parser = generated::Xsmpsed::create_parser();
    body = comment(root) + withParameters("schedule");
    for (const auto* key : {"EpochTime", "MissionStart"})
      if (auto value = field(root, key); !value.empty())
        body += std::string(key) == "EpochTime" ? " epoch " + quote(value)
                                                : " mission " + quote(value);
    body += '\n';
    for (auto c : root.children()) {
      if (local(c.name()) == "Task") {
        std::string activities;
        for (auto a : c.children())
          if (local(a.name()) == "Activity")
            activities +=
                docComment(text(a, "Description")) + reader.scheduleActivity(a);
        body += comment(c)
                + block("task " + identifier(attr(c, "Name", "__task__")),
                        activities);
      } else if (local(c.name()) == "Event")
        body += docComment(text(c, "Description")) + reader.scheduleEvent(c);
    }
  } else
    throw std::runtime_error("Unsupported SMP XML root '" + rootName + "'");

  // A rendering that does not parse is an error, not a file to write.
  const auto syntax = parser->parse(body);
  if (!syntax.accepted || !syntax.value || !syntax.diagnostics.empty()) {
    std::string detail;
    for (const auto& d : syntax.diagnostics)
      detail += " " + d.message + " at " + std::to_string(d.begin) + ": "
                + body.substr(d.begin > 35 ? d.begin - 35 : 0, 100);
    throw std::runtime_error("Imported SMP document does not parse as "
                             + extension + ":" + detail);
  }

  result.content = notice + body;
  result.outputPath = input;
  result.outputPath.replace_extension(extension);
  result.warnings = std::move(reader.warnings);
  return result;
}

ImportResult importFile(const std::filesystem::path& input,
                        std::filesystem::path output, bool overwrite,
                        const ImportOptions& options) {
  auto result = renderImportedDocument(input, options);
  if (!output.empty()) result.outputPath = std::move(output);
  if (!overwrite && std::filesystem::exists(result.outputPath))
    throw std::runtime_error("Refusing to overwrite existing file '"
                             + result.outputPath.string() + "'");
  writeFileIfChanged(result.outputPath, result.content);
  return result;
}

}  // namespace xsmp
