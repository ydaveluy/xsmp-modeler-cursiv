#include <algorithm>
#include <cctype>
#include <cursiv/completion/FuzzyMatcher.hpp>
#include <cursiv/documentation/DocumentationProvider.hpp>
#include <cursiv/parser/Parser.hpp>
#include <cursiv/references/ScopeProvider.hpp>
#include <cursiv/syntax-tree/AstReflection.hpp>
#include <cursiv/syntax-tree/CstUtils.hpp>
#include <cursiv/workspace/Document.hpp>
#include <deque>
#include <set>
#include <unordered_set>
#include <xsmp/DeploymentCompletionProvider.hpp>
#include <xsmp/DeploymentSemantics.hpp>
#include <xsmp/references/QualifiedName.hpp>
#include <xsmp/solver/Solver.hpp>
#include <xsmp/utils/XsmpDocumentation.hpp>
#include <xsmp/utils/XsmpUtils.hpp>

namespace xsmp {

namespace {

using Node = cursiv::AstNode;

// The keywords of a statement or a value: a snippet or a typed value stands for
// them, never the bare word.
bool writtenByItsItem(std::string_view keyword) {
  static const std::set<std::string_view> keywords = {
      "assembly", "configuration", "link",      "schedule",  "task",
      "event",    "include",       "configure", "subscribe", "property",
      "call",     "field",         "interface", "transfer",  "trig",
      "execute",  "emit",          "true",      "false"};
  return keywords.contains(keyword);
}

// Whether `text` holds a tab stop, and so must be sent as a snippet.
bool snippetText(std::string_view text) {
  for (auto at = text.find('$'); at != text.npos; at = text.find('$', at + 1))
    if (at + 1 < text.size()
        && (text[at + 1] == '{'
            || std::isdigit(static_cast<unsigned char>(text[at + 1]))))
      return true;
  return false;
}

std::string ownerName(const Node& node) {
  const auto* owner = cursiv::ast_cast<NamedElement>(node.parent);
  return owner ? std::string(owner->name) : std::string{};
}

// `name` under the component at `path`, where `.` is the component itself.
std::string qualify(std::string_view path, std::string_view name) {
  return path == "." ? std::string(name)
                     : std::string(path) + "." + std::string(name);
}

std::string placeholderText(unsigned index, std::string_view text) {
  std::string escaped;
  for (const auto c : text) {
    if (c == '$' || c == '}' || c == '\\') escaped += '\\';
    escaped += c;
  }
  return "${" + std::to_string(index) + ":" + escaped + "}";
}

// The value a new assignment of this type starts from, as the deployment
// languages write it. Empty where the type has none. Arrays and structures are
// spelled out only under `composite`, two levels deep at most and only while
// they stay short.
std::string defaultValue(const Type* type, bool composite = true,
                         unsigned depth = 0) {
  constexpr std::size_t maxSize = 4096;
  if (!type) return {};

  if (composite && depth < 2) {
    if (const auto* array = cursiv::ast_cast<Array>(type)) {
      const auto value = getValue(array->size);
      const auto size = value ? integralValueOf(*value) : std::nullopt;
      if (!size || *size <= 0) return "[]";
      std::string out = "[",
                  item = defaultValue(array->itemType.get(), true, depth + 1);
      if (item.empty()) item = "\"\"";
      if (static_cast<std::uint64_t>(*size) * (item.size() + 2) > maxSize)
        return "[]";
      for (std::int64_t i = 0; i < *size; ++i) {
        if (i) out += ", ";
        out += item;
      }
      return out + "]";
    }

    if (const auto* structure = cursiv::ast_cast<Structure>(type)) {
      std::string out = "{ ";
      for (const auto* field : getAllFields(*structure)) {
        if (out.size() > 2) out += ", ";
        const auto value = defaultValue(field->type.get(), true, depth + 1);
        out +=
            std::string(field->name) + " = " + (value.empty() ? "\"\"" : value);
        if (out.size() > maxSize) return "{ }";
      }
      return out + " }";
    }
  }

  if (const auto* enumeration = cursiv::ast_cast<Enumeration>(type))
    return enumeration->literal.empty()
               ? ""
               : qualifiedName(*enumeration->literal[0]);

  switch (getPTK(type)) {
    case PTK::Bool:
      return "false";
    case PTK::Char8:
      return "'A'";
    case PTK::String8:
      return "\"\"";
    case PTK::DateTime:
      return "\"1970-01-01T00:00:00Z\"dt";
    case PTK::Duration:
      return "\"PT0S\"d";
    case PTK::Float32:
    case PTK::Float64:
      return "0.0";
    default:
      return isIntegralType(getPTK(type)) ? "0" : "";
  }
}

// The scalar start of `name = ...`: `value` where the type has no default.
std::string assignmentValue(const Type* type) {
  const auto value = defaultValue(type, false);
  return value.empty() ? "value" : value;
}

// The block the caret writes in: climbing from the last token, the innermost
// one that opened its brace and has not closed it yet.
const Node* openBlock(const cursiv::CompletionContext& context) {
  const auto& cst = context.document.parseResult.cst();
  const Node* last = nullptr;
  for (auto id = cst.leaf_after(context.offset); id > 0 && !last;) {
    const auto leaf = cst.as_leaf(id = cst.leaf_before(id));
    if (!leaf.hidden && !leaf.recovered)
      last = cursiv::owner_of_leaf(cst, leaf.id);
  }

  for (const auto* at = last; at; at = at->parent) {
    if (!(cursiv::ast_cast<ModelInstance>(at)
          || cursiv::ast_cast<AssemblyComponentConfiguration>(at)
          || cursiv::ast_cast<ComponentConfiguration>(at)
          || cursiv::ast_cast<ComponentLinkBase>(at)
          || cursiv::ast_cast<Task>(at) || cursiv::ast_cast<StructureValue>(at))
        || at->cstNodeId == cursiv::kNoNode)
      continue;

    const auto& span = cst[at->cstNodeId];
    auto opened = false, closed = false;
    for (auto id = cst.leaf_after(span.begin); id < cst.size();
         id = cst.next_leaf(id)) {
      const auto leaf = cst.as_leaf(id);
      if (leaf.begin < span.begin) continue;
      if (leaf.begin >= span.end || leaf.begin >= context.offset) break;
      if (leaf.hidden || leaf.recovered) continue;
      opened |= cst.text(leaf.id) == "{";
      closed = leaf.end == span.end && cst.text(leaf.id) == "}";
    }

    if (opened && !closed) return at;
  }
  return nullptr;
}

// The nearest node around `node` that holds or assigns a value: it says what
// type the caret expects.
const Node* valueOwner(const Node* node) {
  for (; node; node = node->parent)
    if (cursiv::ast_cast<FieldValue>(node)
        || cursiv::ast_cast<CfgStructureFieldValue>(node)
        || cursiv::ast_cast<PropertyValue>(node)
        || cursiv::ast_cast<SetProperty>(node)
        || cursiv::ast_cast<ParameterValue>(node)
        || cursiv::ast_cast<ArrayValue>(node)
        || cursiv::ast_cast<StructureValue>(node))
      return node;
  return nullptr;
}

// `name(parameter = value, ...)`, with a tab stop on each value.
std::string operationText(const Operation& operation) {
  std::string out(operation.name);
  out += '(';
  unsigned index = 0;
  for (const auto* parameter : operation.parameter) {
    if (!parameter) continue;
    if (index) out += ", ";
    const auto value = defaultValue(parameter->type.get(), false);
    out += std::string(parameter->name) + " = "
           + placeholderText(++index, value.empty() ? "value" : value);
  }
  return out + ")";
}

}  // namespace

::lsp::CompletionList DeploymentCompletionProvider::getCompletion(
    const cursiv::Document& document, const ::lsp::CompletionParams& params,
    const cursiv::CancellationToken& token) const {
  auto list = DefaultCompletionProvider::getCompletion(document, params, token);
  std::ranges::stable_sort(list.items, {}, [](const auto& i) {
    return i.sortText.value_or(i.label);
  });
  return list;
}

bool DeploymentCompletionProvider::writes(const Context& context,
                                          const cursiv::AstTypeTag* type,
                                          std::string_view property) {
  return std::ranges::any_of(context.features, [&](const auto& next) {
    return next.type == type && next.property == property;
  });
}

bool DeploymentCompletionProvider::spells(const cursiv::NextFeature& next,
                                          std::string_view keyword) const {
  const auto spelled =
      services.parser->getGrammar().terminalSpellings(next.feature);
  return std::ranges::find(spelled, keyword) != spelled.end();
}

void DeploymentCompletionProvider::completionFor(
    const Context& context, const cursiv::NextFeature& next,
    Accept accept) const {
  // Any value may start where the grammar can read `true`.
  const auto value = writes(context, &cursiv::ast_type_tag<BoolValue>, "value");
  const auto structure =
      writes(context, &cursiv::ast_type_tag<StructureValue>, "elements");
  const auto statement = !value && !structure && atStatement(context);

  // Once for this reading of the caret.
  if (&next == context.features.data()) {
    if (structure)
      completeStructure(context, accept);
    else if (value)
      completeValue(context, accept);
    else if (statement)
      completeStatement(context, openBlock(context), accept);
  }

  if (structure || value) return;

  const auto spelled =
      services.parser->getGrammar().terminalSpellings(next.feature);
  const auto name =
      !next.property.empty() && next.type
      && services.astReflection->referenceIndexOf(next.type, next.property)
             != cursiv::kNoReference;

  // A keyword the grammar lets a name spell here says nothing more of the name.
  if (name && !spelled.empty()) return;
  // At the start of a statement, the statement itself says what its names mean.
  if (statement && next.type == &cursiv::ast_type_tag<ConcretePathNamedSegment>)
    return;

  DefaultCompletionProvider::completionFor(context, next, accept);
}

void DeploymentCompletionProvider::visitReferenceCandidates(
    const cursiv::ReferenceInfo& reference, const Context& context,
    cursiv::function_ref<bool(const cursiv::AstNodeDescription&)> visitor)
    const {
  const auto* segment =
      cursiv::ast_cast<ConcretePathNamedSegment>(reference.container);
  // Only a path segment is read from the model; any other name is the scope's.
  if (!segment || cursiv::ast_cast<CfgStructureFieldValue>(segment->parent)) {
    // An assembly does not instantiate itself.
    const auto* self = cursiv::ast_cast<AssemblyInstance>(reference.container)
                           ? context.document.parseResult.value
                           : nullptr;
    DefaultCompletionProvider::visitReferenceCandidates(
        reference, context, [&](const cursiv::AstNodeDescription& candidate) {
          return (self && candidate.node == self) || visitor(candidate);
        });
    return;
  }

  // The segment the caret writes is the one being typed, or extends the path
  // before it, or starts one.
  const auto* typed = cursiv::ast_cast<PathSegment>(segment->parent);
  const auto* wanted = typed ? typed : segment;
  // A path not written yet: a throwaway one says where it would start.
  Path start;
  start.type = &cursiv::ast_type_tag<Path>;
  start.parent = segment->parent;
  const auto* path = cursiv::container_of_type<Path>(wanted);
  const auto* owner =
      path ? ownerOfSegment(wanted).node : pathContextResolution(&start).node;

  // The first path of a link is the one no arrow stands before.
  auto first = true;
  const auto& cst = context.document.parseResult.cst();
  for (auto id = cst.leaf_after(context.offset); id > 0 && first;) {
    const auto leaf = cst.as_leaf(id = cst.leaf_before(id));
    if (leaf.hidden) continue;
    if (cst.text(leaf.id) == "->") first = false;
    if (cst.text(leaf.id) == "link" || cst.text(leaf.id) == "transfer") break;
  }

  const auto role = roleAt(path ? path->parent : start.parent, first);

  // A placeholder of a name is read through the template of its owner.
  std::deque<std::string> concrete;
  (void)xsmp::members(owner, [&](const AstNode* node) {
    if (!roleAccepts(role, node) && !cursiv::ast_cast<ModelInstance>(node)
        && !cursiv::ast_cast<AssemblyInstance>(node))
      return true;
    const auto* description = descriptionOf(node);
    if (!description) return true;
    auto candidate = *description;
    if (const auto* named = cursiv::ast_cast<NamedElement>(node))
      if (const auto name = concreteName(*named, owner))
        candidate.name = concrete.emplace_back(*name);
    return visitor(candidate);
  });
}

cursiv::CompletionValueItem
DeploymentCompletionProvider::createReferenceCompletionItem(
    const cursiv::AstNodeDescription& candidate,
    const cursiv::ReferenceInfo& reference, const Context& context) const {
  auto item = DefaultCompletionProvider::createReferenceCompletionItem(
      candidate, reference, context);
  item.label = std::string(candidate.name);
  item.filterText = item.label;
  item.sortText = "1000_" + item.label;

  const auto* node = candidate.node;
  if (node && docTag(*node, "deprecated")) {
    item.tags.emplace();
    item.tags->emplace_back(::lsp::CompletionItemTag::Deprecated);
  }

  // Turns the item into a value that inserts `content`, not just the name.
  const auto write = [&](std::string content, std::string detail) {
    item.kind = ::lsp::CompletionItemKind::Value;
    item.sortText = "2000_" + item.label;
    if (snippetText(content))
      item.insertTextFormat = ::lsp::InsertTextFormat::Snippet;
    item.textEdit = buildCompletionTextEdit(context, std::move(content));
    item.detail = std::move(detail);
  };

  const auto name = item.label;
  // What the statement around the reference makes of each kind of member.
  const auto* site = reference.container->parent;
  if (reference.property == "implementation") {
    item.insertText = name;
    item.detail = "Component implementation type.";
  } else if (reference.property == "assembly"
             && cursiv::ast_cast<AssemblyInstance>(reference.container)) {
    item.insertText = name;
    item.detail = "Assembly type.";
  } else if (const auto* task = cursiv::ast_cast<Task>(node);
             task && reference.property == "task"
             && cursiv::ast_cast<ExecuteTask>(reference.container)) {
    write(name + executeText(*task).substr(task->name.size()),
          "Schedule task " + name + ".");
  } else if (const auto* operation = cursiv::ast_cast<Operation>(node)) {
    write(operationText(*operation), "Operation of " + ownerName(*node) + ".");
  } else if (const auto* property = cursiv::ast_cast<Property>(node)) {
    write(name + " = " + placeholder(1, assignmentValue(property->type.get())),
          std::string(cursiv::ast_cast<PropertyValue>(site)
                          ? "Writable property of "
                          : "Property of ")
              + ownerName(*node) + ".");
  } else if (cursiv::ast_cast<EntryPoint>(node)) {
    const bool subscription = cursiv::ast_cast<GlobalEventHandler>(site);
    write(name + (subscription ? " -> \"${1:GlobalEvent}\"" : ""),
          subscription ? "Entry point of " + ownerName(*node) + "."
                       : "Entry point of the current component.");
  } else if (cursiv::ast_cast<Reference>(node)) {
    write(name, "Reference of " + ownerName(*node) + ".");
  } else if (cursiv::ast_cast<ModelInstance>(node)
             || cursiv::ast_cast<AssemblyInstance>(node)) {
    write(name, "Child instance of the current assembly.");
  } else if (const auto* field = cursiv::ast_cast<Field>(node);
             field && cursiv::container_of_type<FieldValue>(site)) {
    write(name + " = " + placeholder(1, assignmentValue(field->type.get())),
          "Field of " + ownerName(*node) + ".");
  }

  return item;
}

void DeploymentCompletionProvider::completionForKeyword(
    const Context& context, const cursiv::NextFeature& next,
    std::string_view keyword, Accept accept) const {
  completeKeyword(context, openBlock(context), keyword, accept);
  if (writtenByItsItem(keyword)) return;
  DefaultCompletionProvider::completionForKeyword(
      context, next, keyword,
      [&](const Context& at, cursiv::CompletionValueItem&& item) {
        item.filterText = item.label;
        item.sortText = "4000_" + item.label;
        return accept(at, std::move(item));
      });
}

void DeploymentCompletionProvider::offerSnippet(const Context& context,
                                                Accept accept,
                                                std::string label,
                                                std::string body,
                                                std::string detail) const {
  const auto& matcher = *services.lspShared.fuzzyMatcher;
  if (!matcher.match(context.identifier, label)
      && !matcher.match(context.identifier, body))
    return;

  cursiv::CompletionValueItem item;
  item.detail = std::move(detail);
  item.kind = ::lsp::CompletionItemKind::Snippet;
  item.insertTextFormat = ::lsp::InsertTextFormat::Snippet;
  item.textEdit = buildCompletionTextEdit(context, body);
  item.insertText = std::move(body);
  item.filterText = label;
  item.sortText = "3000_" + label;
  item.label = std::move(label);
  (void)accept(context, std::move(item));
}

bool DeploymentCompletionProvider::offerValue(const Context& context,
                                              Accept accept, std::string label,
                                              std::string content,
                                              std::string detail,
                                              const Node* node) const {
  cursiv::throw_if_cancelled(context.cancelToken);
  const auto& matcher = *services.lspShared.fuzzyMatcher;
  if (!matcher.match(context.identifier, label)
      && !matcher.match(context.identifier, content))
    return true;

  cursiv::CompletionValueItem item;
  item.detail = std::move(detail);
  item.kind = ::lsp::CompletionItemKind::Value;
  if (snippetText(content))
    item.insertTextFormat = ::lsp::InsertTextFormat::Snippet;
  item.textEdit = buildCompletionTextEdit(context, std::move(content));
  item.filterText = label;
  item.sortText = "2000_" + label;
  item.label = std::move(label);

  if (node) {
    if (const auto docs =
            services.documentationProvider->getDocumentation(*node))
      item.documentation = ::lsp::MarkupContent{
          .kind = ::lsp::MarkupKind::Markdown, .value = *docs};
    if (docTag(*node, "deprecated")) {
      item.tags.emplace();
      item.tags->emplace_back(::lsp::CompletionItemTag::Deprecated);
    }
  }
  return accept(context, std::move(item));
}

void DeploymentCompletionProvider::offerTyped(const Context& context,
                                              Accept accept, std::string label,
                                              std::string content,
                                              std::string detail,
                                              const Node* node) const {
  (void)offerValue(
      context,
      [&](const Context& at, cursiv::CompletionValueItem&& item) {
        std::visit([&](const auto& edit) { item.insertText = edit.newText; },
                   *item.textEdit);
        return accept(at, std::move(item));
      },
      std::move(label), std::move(content), std::move(detail), node);
}

// What the type a value is expected to have admits.
void DeploymentCompletionProvider::completeValue(const Context& context,
                                                 Accept accept) const {
  // No value stands at the caret yet: a throwaway child of its owner asks for
  // the type one would have.
  Value probe;
  probe.parent = const_cast<Node*>(valueOwner(context.node()));
  const auto* expected =
      cursiv::ast_cast<Type>(inferDeploymentValueType(&probe));
  if (!expected) return;

  const auto name = qualifiedName(*expected);
  const auto offer = [&](std::string label, std::string content,
                         std::string detail, const Node* node = nullptr) {
    offerTyped(context, accept, std::move(label), std::move(content),
               std::move(detail), node);
  };

  if (const auto* enumeration = cursiv::ast_cast<Enumeration>(expected))
    for (const auto* literal : enumeration->literal)
      offer(qualifiedName(*literal), qualifiedName(*literal),
            "Enumeration literal of " + name + ".", literal);

  switch (getPTK(expected)) {
    case PTK::Bool:
      offer("false", "false", "Boolean value for " + name + ".");
      offer("true", "true", "Boolean value for " + name + ".");
      break;
    case PTK::Char8:
      offer("'A'", "'A'", "Character value for " + name + ".");
      break;
    case PTK::String8:
      offer("\"\"", "\"\"", "String value for " + name + ".");
      break;
    case PTK::Duration:
      offer("\"PT0S\"d", "\"PT0S\"d", "Duration value for " + name + ".");
      break;
    case PTK::DateTime:
      offer("\"1970-01-01T00:00:00Z\"dt", "\"1970-01-01T00:00:00Z\"dt",
            "DateTime value for " + name + ".");
      break;
    default:
      break;
  }

  // A composite value is offered where the grammar reads one.
  const auto composite = std::ranges::any_of(
      context.features,
      [&](const cursiv::NextFeature& next) { return spells(next, "["); });
  const auto value = defaultValue(expected, composite);
  if (composite && cursiv::ast_cast<Array>(expected))
    offer(value, value, "Array value for " + name + ".");
  if (composite && cursiv::ast_cast<Structure>(expected))
    offer("{...}", value, "Structure value for " + name + ".");
  if (!value.empty())
    offer("Default Value", value, "Default value for " + name + ".");
}

void DeploymentCompletionProvider::completeStructure(const Context& context,
                                                     Accept accept) const {
  const auto* structure =
      cursiv::ast_cast<StructureValue>(valueOwner(context.node()));
  if (!structure) return;
  for (const auto* member : members(inferDeploymentValueType(structure)))
    if (const auto* field = cursiv::ast_cast<Field>(member)) {
      const auto value = defaultValue(field->type.get());
      if (!offerValue(context, accept, std::string(field->name),
                      std::string(field->name) + " = "
                          + placeholder(1, value.empty() ? "value" : value),
                      "Field of the current Structure value.", field))
        return;
    }
}

std::vector<std::string> DeploymentCompletionProvider::names(
    const Context& context, const cursiv::AstTypeTag* type,
    std::string_view property) const {
  std::vector<std::string> result;
  std::set<std::string> seen;
  const auto add = [&](std::string_view name) {
    if (!name.empty() && seen.emplace(name).second) result.emplace_back(name);
  };

  // A throwaway node of this class under the caret's asks the scope.
  auto node = services.astReflection->createNode(type);
  if (!node) return result;
  const auto* parent = context.node();
  node->parent =
      const_cast<Node*>(parent ? parent : context.document.parseResult.value);
  cursiv::ReferenceInfo reference{.container = node.get(),
                                  .document = &context.document,
                                  .property = property};
  reference.reference =
      services.astReflection->referenceIndexOf(type, property);
  (void)services.scopeProvider->visitScopeEntries(reference,
                                                  [&](const auto& entry) {
                                                    add(entry.name);
                                                    return true;
                                                  });
  return result;
}

void DeploymentCompletionProvider::offerFieldAssignments(
    const Context& context, Accept accept, const Node* component,
    std::string_view detailPrefix) const {
  const auto* type = cursiv::ast_cast<Type>(targetType(component));
  for (const auto* member : members(component))
    if (const auto* field = cursiv::ast_cast<Field>(member);
        field && !field->name.empty()) {
      const auto value = defaultValue(field->type.get());
      if (!offerValue(context, accept, std::string(field->name),
                      std::string(field->name) + " = "
                          + placeholder(1, value.empty() ? "value" : value),
                      std::string(detailPrefix)
                          + (type ? std::string(type->name) : ownerName(*field))
                          + ".",
                      field))
        return;
    }
}

void DeploymentCompletionProvider::offerInvocations(const Context& context,
                                                    Accept accept,
                                                    const Node* component,
                                                    bool globalEvents) const {
  for (const auto* member : members(component)) {
    const std::string name(member->name);
    if (name.empty()) continue;
    if (const auto* operation = cursiv::ast_cast<Operation>(member);
        operation
        && !offerValue(context, accept, "call " + name,
                       "call " + operationText(*operation),
                       "Operation call for " + name + ".", operation))
      return;
    if (const auto* property = cursiv::ast_cast<Property>(member);
        property
        && !offerValue(
            context, accept, "property " + name,
            "property " + name + " = "
                + placeholder(1, assignmentValue(property->type.get())),
            "Property value for " + name + ".", property))
      return;
    if (const auto* entry = cursiv::ast_cast<EntryPoint>(member);
        entry
        && !offerValue(
            context, accept, (globalEvents ? "subscribe " : "trig ") + name,
            globalEvents ? "subscribe " + name + " -> \"${1:GlobalEvent}\""
                         : "trig " + name,
            (globalEvents ? "Global event subscription for entry point "
                          : "Trigger entry point ")
                + name + ".",
            entry))
      return;
  }
}

namespace {

// A member a link can connect, with the path of its component.
struct Port {
  std::string_view path;
  const NamedElement* node;
};

// The members of some components, by the role they take in a link.
struct Ports {
  std::vector<Port> outputs, inputs, sources, sinks;
};

// A template because `RelativeComponent` is protected, out of reach here.
template <class RelativeComponents>
Ports portsOf(const RelativeComponents& relative) {
  Ports ports;
  for (const auto& component : relative)
    (void)visitMembers(component.node, [&](const NamedElement& member) {
      const Port port{component.path, &member};
      if (const auto* field = cursiv::ast_cast<Field>(&member)) {
        if (hasModifier(*field, "output")) ports.outputs.push_back(port);
        if (hasModifier(*field, "input")) ports.inputs.push_back(port);
      }
      if (cursiv::ast_cast<EventSource>(&member)) ports.sources.push_back(port);
      if (cursiv::ast_cast<EventSink>(&member)) ports.sinks.push_back(port);
      return true;
    });

  return ports;
}

}  // namespace

std::string DeploymentCompletionProvider::linkEnds(
    const std::vector<RelativeComponent>& relative, bool events,
    std::string_view from, std::string_view to) {
  const auto ports = portsOf(relative);
  const auto end = [](unsigned index, const std::vector<Port>& members,
                      std::string_view fallback) {
    std::vector<std::string> paths;
    for (const auto& port : members)
      if (!port.node->name.empty())
        paths.push_back(qualify(port.path, port.node->name));
    return choice(index, paths, fallback);
  };
  return end(1, events ? ports.sources : ports.outputs, from) + " -> "
         + end(2, events ? ports.sinks : ports.inputs, to);
}

void DeploymentCompletionProvider::offerInterfaceLinks(
    const Context& context, Accept accept,
    const std::vector<RelativeComponent>& relative) const {
  for (const auto& owner : relative)
    for (const auto* node : members(owner.node)) {
      const auto* reference = cursiv::ast_cast<Reference>(node);
      if (!reference) continue;
      for (const auto& client : relative) {
        if (owner.path == client.path) continue;
        const auto* expected = reference->interface_.get();
        const auto* actual = cursiv::ast_cast<Type>(targetType(client.node));
        if (expected
            && !isBaseOfReferenceType(expected, qualifiedName(*expected),
                                      actual))
          continue;

        // The first reference of the client that accepts the owner back.
        std::string back;
        for (const auto* candidate : members(client.node))
          if (const auto* reverse = cursiv::ast_cast<Reference>(candidate);
              reverse && reverse->interface_.get()
              && isBaseOfReferenceType(
                  reverse->interface_.get(),
                  qualifiedName(*reverse->interface_.get()),
                  cursiv::ast_cast<Type>(targetType(owner.node)))) {
            back = ":" + std::string(reverse->name);
            break;
          }

        const auto label = "interface link "
                           + qualify(owner.path, reference->name) + " -> "
                           + client.path + back;
        if (!offerValue(context, accept, label, label, "Interface Link"))
          return;
      }
    }
}

const Node* DeploymentCompletionProvider::componentOf(const Node* block) {
  const Node* component = nullptr;
  if (cursiv::ast_cast<ModelInstance>(block)) component = block;
  if (const auto* config = cursiv::ast_cast<ComponentConfiguration>(block)) {
    component = config->context.get<Component>();
    if (!component) component = config->context.get<Assembly>();
    if (!component) component = pathTarget(config->name);
  }
  if (const auto* config =
          cursiv::ast_cast<AssemblyComponentConfiguration>(block))
    component = pathTarget(config->name);
  if (const auto* link = cursiv::ast_cast<ComponentLinkBase>(block))
    component = pathTarget(link->name);
  if (const auto* task = cursiv::ast_cast<Task>(block)) {
    component = task->context.get<Component>();
    if (!component) component = task->context.get<Assembly>();
  }
  if (const auto* assembly = cursiv::ast_cast<Assembly>(component))
    component = assembly->model;
  return component;
}

std::vector<DeploymentCompletionProvider::RelativeComponent>
DeploymentCompletionProvider::relativeComponents(const Node* component,
                                                 bool nested) {
  std::vector<RelativeComponent> result{{.path = ".", .node = component}};
  std::unordered_set<const Node*> ancestry;
  // `ancestry` and `depth` stop an assembly that instantiates itself.
  const auto descend = [&](auto&& self, const Node* node, std::string prefix,
                           unsigned depth) -> void {
    if (!node || depth > 32 || !ancestry.insert(node).second) return;
    for (const auto* child : members(node))
      if (cursiv::ast_cast<ModelInstance>(child)
          || cursiv::ast_cast<AssemblyInstance>(child)) {
        const auto name =
            concreteName(*child, node).value_or(std::string(child->name));
        const auto path = prefix.empty() ? name : prefix + "." + name;
        result.push_back({.path = path, .node = child});
        self(self, child, path, depth + 1);
      }
    ancestry.erase(node);
  };

  if (nested) descend(descend, component, {}, 0);
  return result;
}

std::vector<const NamedElement*> DeploymentCompletionProvider::members(
    const Node* owner) {
  std::vector<const NamedElement*> result;
  visitMembers(owner, [&](const NamedElement& node) {
    result.push_back(&node);
    return true;
  });
  return result;
}

std::string DeploymentCompletionProvider::placeholder(unsigned index,
                                                      std::string_view text) {
  return placeholderText(index, text);
}

std::string DeploymentCompletionProvider::choice(
    unsigned index, const std::vector<std::string>& values,
    std::string_view fallback) {
  std::unordered_set<std::string_view> seen;
  std::string out;
  for (const auto& value : values) {
    if (value.empty() || !seen.insert(value).second) continue;
    out += out.empty() ? "${" + std::to_string(index) + "|" : ",";
    for (const auto c : value) {
      if (c == ',' || c == '|' || c == '\\') out += '\\';
      out += c;
    }
  }
  return out.empty() ? placeholder(index, fallback) : out + "|}";
}

std::string DeploymentCompletionProvider::executeText(const Task& task) {
  const auto* schedule = cursiv::container_of_type<Schedule>(task.parent);
  std::string out(task.name);
  if (!schedule || schedule->parameters.empty()) return out;
  out += '<';
  unsigned index = 0;
  for (const auto* parameter : schedule->parameters) {
    if (index) out += ", ";
    std::string value = "value";
    if (const auto* text = cursiv::ast_cast<StringParameter>(parameter))
      value = text->value.data() ? std::string(text->value) : "\"value\"";
    else if (const auto* number = cursiv::ast_cast<Int32Parameter>(parameter))
      value = std::to_string(number->value.value_or(0));
    out += std::string(parameter->name) + " = " + placeholder(++index, value);
  }
  return out + "> at " + placeholder(index + 1, "path");
}

}  // namespace xsmp
