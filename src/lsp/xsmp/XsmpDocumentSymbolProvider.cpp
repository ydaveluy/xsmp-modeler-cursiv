#include <algorithm>
#include <cursiv/references/NameProvider.hpp>
#include <cursiv/symbols/NodeKindProvider.hpp>
#include <cursiv/syntax-tree/AstReflection.hpp>
#include <cursiv/text/LspPosition.hpp>
#include <functional>
#include <xsmp/AttributeHelper.hpp>
#include <xsmp/XsmpDocumentSymbolProvider.hpp>
#include <xsmp/ast.hpp>
#include <xsmp/solver/Solver.hpp>
#include <xsmp/utils/XsmpDocumentation.hpp>
#include <xsmp/utils/XsmpUtils.hpp>

namespace xsmp {

namespace {

using Node = cursiv::AstNode;

std::string quote(std::string_view text) {
  return "\"" + cppEscaped(text) + "\"";
}

std::string segment(const PathSegment* n) {
  if (auto* p = cursiv::ast_cast<ConcretePathNamedSegment>(n))
    return std::string(p->reference.refText);
  if (auto* p = cursiv::ast_cast<PatternPathNamedSegment>(n))
    return std::string(p->text);
  if (cursiv::ast_cast<PathSelfSegment>(n)) return ".";
  if (cursiv::ast_cast<PathParentSegment>(n)) return "..";
  return {};
}

std::string path(const Path* n) {
  if (!n) return {};
  std::string out = n->absolute ? "/" : "";
  out += segment(n->head);
  for (auto* element : n->elements) {
    if (auto* member = cursiv::ast_cast<PathMember>(element))
      out += std::string(member->separator) + segment(member->segment);
    if (auto* index = cursiv::ast_cast<PathIndex>(element))
      out += "[" + std::to_string(index->index) + "]";
  }
  return out;
}

std::string local(const LocalNamedReference* n) {
  return n ? (n->unsafe ? "unsafe " : "") + std::string(n->reference.refText)
           : "<unknown>";
}

// The name of what a reference resolves to, otherwise what is written.
template <class... Ts>
std::string reference(const cursiv::reference<Ts...>& ref) {
  if (ref.resolve() == cursiv::RefState::Resolved)
    if (const auto* target = ref.target();
        target && target->type && target->type->readName)
      return std::string(target->type->readName(*target));
  return ref.refText.empty() ? "<unknown>" : std::string(ref.refText);
}

// `(a, b)`: the names of the parameters a call sets.
std::string arguments(const std::vector<ParameterValue*>& parameters) {
  std::string out = "(";
  for (auto* parameter : parameters)
    if (parameter && !parameter->parameter.empty()) {
      if (out.size() > 1) out += ", ";
      out += parameter->parameter;
    }
  return out + ")";
}

// An operation reads with its parameter types, which tells overloads apart.
std::string signature(const NamedElement& node) {
  const auto* operation = cursiv::ast_cast<Operation>(&node);
  if (!operation) return std::string(node.name);
  std::string out(node.name);
  out += '(';
  for (auto* parameter : operation->parameter) {
    if (!parameter) continue;
    if (out.back() != '(') out += ", ";
    if (isConst(*parameter)) out += "const ";
    out += parameter->type.get() ? qualifiedName(*parameter->type.get())
                                 : std::string(parameter->type.refText);
    if (isByPointer(*parameter)) out += '*';
    if (isByReference(*parameter)) out += '&';
  }
  return out + ")";
}

// The name of a node in the outline. Empty for a node with no symbol of its
// own, whose children then stand in its place.
std::string symbolName(const Node& node) {
  if (auto* instance = cursiv::ast_cast<ModelInstance>(&node)) {
    if (instance->name.empty()) return {};
    if (auto* sub = cursiv::ast_cast<SubInstance>(node.parent))
      return local(sub->container) + " += " + std::string(instance->name);
    return std::string(instance->name);
  }
  if (auto* instance = cursiv::ast_cast<AssemblyInstance>(&node)) {
    if (instance->name.empty()) return {};
    if (auto* sub = cursiv::ast_cast<SubInstance>(node.parent))
      return local(sub->container) + " += " + std::string(instance->name);
    return std::string(instance->name);
  }
  if (auto* cfg = cursiv::ast_cast<AssemblyComponentConfiguration>(&node))
    return "configure " + path(cfg->name);
  if (auto* cfg = cursiv::ast_cast<ComponentConfiguration>(&node))
    return path(cfg->name)
           + (cfg->context.refText.empty()
                  ? ""
                  : ": " + std::string(cfg->context.refText));
  if (auto* links = cursiv::ast_cast<ComponentLinkBase>(&node))
    return path(links->name);
  if (auto* usage = cursiv::ast_cast<ConfigurationUsage>(&node))
    return "include " + reference(usage->configuration)
           + (usage->path ? " at " + path(usage->path) : "");
  if (auto* handler = cursiv::ast_cast<GlobalEventHandler>(&node))
    return "subscribe " + local(handler->entryPoint) + " -> "
           + quote(handler->globalEventName);
  if (auto* link = cursiv::ast_cast<InterfaceLink>(&node))
    return "interface link "
           + (path(link->sourcePath).empty() ? "<unknown>"
                                             : path(link->sourcePath))
           + " -> " + path(link->clientPath)
           + (link->backReference
                      && !link->backReference->reference.refText.empty()
                  ? ":" + local(link->backReference)
                  : "");
  if (auto* link = cursiv::ast_cast<EventLink>(&node))
    return "event link " + path(link->ownerPath) + " -> "
           + path(link->clientPath);
  if (auto* link = cursiv::ast_cast<FieldLink>(&node))
    return "field link " + path(link->ownerPath) + " -> "
           + path(link->clientPath);
  if (auto* call = cursiv::ast_cast<OperationCall>(&node))
    return "call " + local(call->operation) + arguments(call->parameters);
  if (auto* call = cursiv::ast_cast<CallOperation>(&node))
    return "call " + path(call->operationPath) + arguments(call->parameters);
  if (auto* property = cursiv::ast_cast<PropertyValue>(&node))
    return "property " + local(property->property);
  if (auto* property = cursiv::ast_cast<SetProperty>(&node))
    return "property " + path(property->propertyPath);
  if (auto* field = cursiv::ast_cast<FieldValue>(&node))
    return path(field->field);
  if (auto* trigger = cursiv::ast_cast<Trigger>(&node))
    return "trig " + path(trigger->entryPoint);
  if (auto* transfer = cursiv::ast_cast<Transfer>(&node))
    return "transfer " + path(transfer->outputFieldPath) + " -> "
           + path(transfer->inputFieldPath);
  if (auto* execute = cursiv::ast_cast<ExecuteTask>(&node))
    return "execute " + reference(execute->task)
           + (execute->root ? " at " + path(execute->root) : "");
  if (auto* emit = cursiv::ast_cast<EmitGlobalEvent>(&node))
    return std::string(emit->asynchronous ? "async " : "") + "emit "
           + quote(emit->eventName);
  if (auto* event = cursiv::ast_cast<MissionEvent>(&node))
    return "event " + reference(event->task) + " mission "
           + quote(event->missionTime);
  if (auto* event = cursiv::ast_cast<EpochEvent>(&node))
    return "event " + reference(event->task) + " epoch "
           + quote(event->epochTime);
  if (auto* event = cursiv::ast_cast<SimulationEvent>(&node))
    return "event " + reference(event->task) + " simulation "
           + quote(event->simulationTime);
  if (auto* event = cursiv::ast_cast<ZuluEvent>(&node))
    return "event " + reference(event->task) + " zulu "
           + quote(event->zuluTime);
  if (auto* event = cursiv::ast_cast<GlobalEventTriggeredEvent>(&node))
    return "event " + reference(event->task) + " on " + quote(event->startEvent)
           + (event->stopEvent.empty() ? ""
                                       : " until " + quote(event->stopEvent));
  if (auto* named = cursiv::ast_cast<NamedElement>(&node))
    return signature(*named);
  return {};
}

// A cardinality as it is written: `?`, `*`, `+`, `[n]` or `[n ... m]`, and
// nothing for exactly one.
std::string multiplicity(const NamedElementWithMultiplicity& node) {
  auto lower = getLower(node).value_or(1), upper = getUpper(node).value_or(1);
  if (lower == 0 && upper == 1) return "?";
  if (lower == 1 && upper == 1) return {};
  if (lower == upper) return "[" + std::to_string(lower) + "]";
  if (upper < 0) {
    if (lower == 0) return "*";
    if (lower == 1) return "+";
    return "[" + std::to_string(lower) + " ... *]";
  }
  return "[" + std::to_string(lower) + " ... " + std::to_string(upper) + "]";
}

// What a symbol shows beside its name: mostly its type as written.
std::optional<std::string> details(const Node& node) {
  if (auto* n = cursiv::ast_cast<Association>(&node))
    return std::string(n->type.refText) + (isByPointer(*n) ? "*" : "");
  if (auto* n = cursiv::ast_cast<Constant>(&node))
    return std::string(n->type.refText);
  if (auto* n = cursiv::ast_cast<Container>(&node))
    return std::string(n->type.refText) + multiplicity(*n);
  if (auto* n = cursiv::ast_cast<EventSink>(&node))
    return "EventSink<" + std::string(n->type.refText) + ">";
  if (auto* n = cursiv::ast_cast<EventSource>(&node))
    return "EventSource<" + std::string(n->type.refText) + ">";
  if (auto* n = cursiv::ast_cast<Field>(&node))
    return std::string(n->type.refText);
  if (auto* n = cursiv::ast_cast<AttributeType>(&node))
    return std::string(n->type.refText);
  if (auto* n = cursiv::ast_cast<AssemblyInstance>(&node))
    return n->assembly.refText.empty()
               ? std::nullopt
               : std::optional(std::string(n->assembly.refText));
  if (auto* n = cursiv::ast_cast<EventType>(&node))
    return n->eventArgs ? std::string(n->eventArgs->refText) : "void";
  if (auto* n = cursiv::ast_cast<Integer>(&node))
    return n->primitiveType ? std::string(n->primitiveType->refText)
                            : "Smp::Int32";
  if (auto* n = cursiv::ast_cast<Float>(&node))
    return n->primitiveType ? std::string(n->primitiveType->refText)
                            : "Smp::Float64";
  if (auto* n = cursiv::ast_cast<ModelInstance>(&node)) {
    if (!n->implementation.refText.empty())
      return std::string(n->implementation.refText);
    return n->strImplementation;
  }
  if (auto* n = cursiv::ast_cast<ValueReference>(&node))
    return std::string(n->type.refText) + "*";
  if (auto* n = cursiv::ast_cast<Array>(&node)) {
    const auto value = getValue(n->size);
    const auto size = value ? integralValueOf(*value) : std::nullopt;
    return std::string(n->itemType.refText) + "["
           + (size ? std::to_string(*size) : "") + "]";
  }
  if (auto* n = cursiv::ast_cast<Operation>(&node))
    return n->returnParameter ? std::string(n->returnParameter->type.refText)
                              : "void";
  if (auto* n = cursiv::ast_cast<Property>(&node))
    return std::string(n->type.refText);
  if (auto* n = cursiv::ast_cast<Reference>(&node))
    return std::string(n->interface_.refText) + multiplicity(*n);
  if (cursiv::ast_cast<Model>(&node) || cursiv::ast_cast<Service>(&node)
      || cursiv::ast_cast<Class>(&node) || cursiv::ast_cast<EntryPoint>(&node))
    return std::string(node.type->name);
  return std::nullopt;
}

// The nodes a symbol holds, in document order, those with no text last.
std::vector<const Node*> children(const Node& node,
                                  const cursiv::LspServices& services) {
  std::vector<const Node*> result;
  if (auto* assembly = cursiv::ast_cast<Assembly>(&node)) {
    for (auto* cfg : assembly->configurations)
      if (cfg) result.push_back(cfg);
    if (assembly->model) result.push_back(assembly->model);
  } else if (auto* sub = cursiv::ast_cast<SubInstance>(&node)) {
    if (sub->instance) result.push_back(sub->instance);
  } else
    services.astReflection->visitMembers(
        node, [&](std::string_view property, std::uint32_t, Node& child) {
          if (property == "elements" || property == "literal")
            result.push_back(&child);
        });

  std::ranges::stable_sort(result, [&](auto* left, auto* right) {
    const auto& cst = node.getDocument()->parseResult.cst();
    return (left->cstNodeId == cursiv::kNoNode ? ~0u
                                               : cst[left->cstNodeId].begin)
           < (right->cstNodeId == cursiv::kNoNode
                  ? ~0u
                  : cst[right->cstNodeId].begin);
  });
  return result;
}

}  // namespace

std::optional<std::string> getXsmpNodeDetails(const cursiv::AstNode& node) {
  return details(node);
}

std::vector<::lsp::DocumentSymbol> XsmpDocumentSymbolProvider::getSymbols(
    const cursiv::Document& document,
    const cursiv::CancellationToken& token) const {
  if (!document.hasAst()) return {};
  const auto collect =
      [&](auto&& self, const Node& node) -> std::vector<::lsp::DocumentSymbol> {
    cursiv::throw_if_cancelled(token);
    std::vector<::lsp::DocumentSymbol> nested;
    for (auto* child : children(node, services)) {
      auto values = self(self, *child);
      nested.insert(nested.end(), std::make_move_iterator(values.begin()),
                    std::make_move_iterator(values.end()));
    }

    const auto name = symbolName(node);
    if (name.empty() || node.cstNodeId == cursiv::kNoNode) return nested;
    const auto& span = document.parseResult.cst()[node.cstNodeId];

    ::lsp::DocumentSymbol symbol;
    symbol.name = name;
    symbol.kind = services.lspShared.nodeKindProvider->getSymbolKind(node);
    symbol.range =
        cursiv::to_lsp(document.textDocument().rangeAt(span.begin, span.end));
    const auto nameNode = services.nameProvider->getNameNode(node, &document);
    symbol.selectionRange =
        nameNode ? cursiv::to_lsp(document.textDocument().rangeAt(
                       nameNode->begin(), nameNode->end()))
                 : symbol.range;
    symbol.children = std::move(nested);
    symbol.detail = details(node);
    if (cursiv::ast_cast<NamedElement>(&node) && docTag(node, "deprecated")) {
      symbol.tags.emplace();
      symbol.tags->emplace_back(::lsp::SymbolTag::Deprecated);
    }
    return {std::move(symbol)};
  };

  return collect(collect, *document.parseResult.value);
}

}  // namespace xsmp
