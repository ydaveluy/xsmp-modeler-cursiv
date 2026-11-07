#pragma once

#include <cstdint>
#include <cursiv/syntax-tree/AstNode.hpp>
#include <cursiv/syntax-tree/AstWalk.hpp>
#include <cursiv/syntax-tree/ReferenceInfo.hpp>
#include <cursiv/utils/FunctionRef.hpp>
#include <cursiv/validation/ValidationAcceptor.hpp>
#include <cursiv/workspace/AstNodeDescription.hpp>
#include <cursiv/workspace/Document.hpp>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>
#include <xsmp/ast.hpp>
#include <xsmp/references/QualifiedName.hpp>

// The meaning shared by the four deployment languages: walking the members of a
// component, resolving paths, expanding template parameters, typing values, and
// the checks built on them.

namespace xsmp {

using cursiv::AstNode;
using cursiv::AstNodeDescription;
using cursiv::ReferenceInfo;
using Bindings = std::map<std::string_view, std::string>;

// The node a path or a segment leads to, null when none, and the values of the
// template parameters at that point. `Bindings` gives a value by parameter
// name.
struct PathResolution {
  const AstNode* node = nullptr;
  Bindings bindings;
};

// What a path has to end on, given where it is written. See `roleOf`.
enum class PathRole {
  Component,
  Field,
  Input,
  Output,
  Source,
  Sink,
  Operation,
  Property,
  EntryPoint,
  Reference,
  Any
};

// What navigating through `node` leads to: the type of a field, the item type
// of an array, the root model of an assembly, the implementation of a model
// instance or else what its container gives. Any other node, a type included,
// is its own target. Null when the target is missing.
const AstNode* targetType(const AstNode* node);

// `elements` is given the own elements of each type, and `yield` each
// sub-instance of a model instance. Each stops the walk by returning false.
template <class Yield, class Elements>
bool membersImpl(const AstNode* node, Yield& yield, Elements& elements,
                 std::unordered_set<const AstNode*>& visited) {
  if (!node || !visited.insert(node).second) return true;
  if (auto* model = cursiv::ast_cast<ModelInstance>(node)) {
    for (auto* element : model->elements)
      if (auto* sub = cursiv::ast_cast<SubInstance>(element);
          sub && sub->instance)
        if (!yield(static_cast<const AstNode*>(sub->instance))) return false;
    return membersImpl(targetType(model), yield, elements, visited);
  }

  const auto* next = targetType(node);
  if (next != node) return membersImpl(next, yield, elements, visited);
  if (auto* component = cursiv::ast_cast<Component>(node)) {
    if (!elements(*node, component->elements)) return false;
    if (component->base
        && !membersImpl(component->base->get(), yield, elements, visited))
      return false;
    for (const auto& base : component->interface_)
      if (!membersImpl(base.get(), yield, elements, visited)) return false;
  } else if (auto* structure = cursiv::ast_cast<Structure>(node)) {
    if (!elements(*node, structure->elements)) return false;
    if (auto* cls = cursiv::ast_cast<Class>(node); cls && cls->base)
      if (!membersImpl(cls->base->get(), yield, elements, visited))
        return false;
  } else if (auto* iface = cursiv::ast_cast<Interface>(node)) {
    if (!elements(*node, iface->elements)) return false;
    for (const auto& base : iface->base)
      if (!membersImpl(base.get(), yield, elements, visited)) return false;
  }

  return true;
}

// Visits what can be named under `node` until `yield` returns false: the
// sub-instances of a model instance, then the members of its type, each type
// before its bases and interfaces. `node` is first followed through
// `targetType`, so a field visits the members of its type. An inheritance cycle
// is walked once. Returns whether the walk went to its end.
template <class Yield>
bool members(const AstNode* node, Yield&& yield) {
  std::unordered_set<const AstNode*> visited;
  const auto elements = [&yield](const AstNode&, const auto& list) {
    for (auto* element : list)
      if (element && !yield(element)) return false;
    return true;
  };
  return membersImpl(node, yield, elements, visited);
}

// The name of any node, through reflection. Empty when it has none.
std::string_view nameOf(const AstNode* node);

// Visits the elements in `list`, those of `type`, written as `name`, until
// `yield` returns false. Returns whether the walk went to its end. A document
// past its local scopes answers from their table.
template <class List, class Yield>
bool elementsNamed(const AstNode& type, const List& list, std::string_view name,
                   Yield&& yield) {
  const auto* document = type.getDocument();
  if (document && !name.empty()
      && document->state.load() >= cursiv::DocumentState::ComputedScopes)
    return document->localSymbols
        .find(&type, name, cursiv::LocalSymbols::hash_name(name))
        .for_each([&](const AstNodeDescription& d) {
          return d.name != name || yield(static_cast<const AstNode*>(d.node));
        });
  for (auto* element : list)
    if (element && nameOf(element) == name && !yield(element)) return false;
  return true;
}

// `members`, limited to those written as `name`.
template <class Yield>
bool membersNamed(const AstNode* node, std::string_view name, Yield&& yield) {
  std::unordered_set<const AstNode*> visited;
  const auto instance = [&](const AstNode* n) {
    return nameOf(n) != name || yield(n);
  };
  const auto elements = [&](const AstNode& type, const auto& list) {
    return elementsNamed(type, list, name, yield);
  };
  return membersImpl(node, instance, elements, visited);
}

// The entry of `node` among the local symbols of its container, which is what a
// scope gives back for it. Null when it has none.
const AstNodeDescription* descriptionOf(const AstNode* node);

// The member of `owner` named `name`, as written or once its placeholders are
// expanded with `bindings`. Null when none.
const AstNode* member(const AstNode* owner, std::string_view name,
                      const Bindings& bindings);

// The type `value` has to have, from where it stands: the field or property it
// sets, the parameter it gives, the item type of its array, the matching field
// of its structure. Null when unknown, or when what it sets is `unsafe`.
const AstNode* inferDeploymentValueType(const Value* value);

// The path as written, placeholders included.
std::string pathText(const Path* path);

// Where `path` starts: the node its first segment is looked up in. An absolute
// path starts at the root of its assembly or of its explicit context, a
// relative one at its nearest enclosing block, and a field designator of a
// structure value at the type of that structure. Null when unknown.
const AstNode* pathContext(const Path* path);

// `pathContext` with the template values at that point: the path of an
// enclosing block may cross assembly instances, which add their arguments.
PathResolution pathContextResolution(const Path* path);

// What `segment` names under `owner`. `first` says it is the head of its path:
// a relative head reads a member, and may name `owner` itself when no member
// fits, while an absolute head names `owner` before any member. Crossing an
// assembly instance adds its arguments to the values.
PathResolution segmentResolution(const PathSegment* segment,
                                 const PathResolution& owner, bool first);

// What `path` names, segment by segment. The node is null when a segment names
// nothing or when an index falls outside its array.
PathResolution resolvePathWithBindings(const Path* path);

const AstNode* pathTarget(const Path* path);

// What `wanted` is looked up in, walking its path up to it. Empty when `wanted`
// is not in a path.
PathResolution ownerOfSegment(const PathSegment* wanted);

// What a local name reads the members of: the client of an interface link for
// its back reference, otherwise the enclosing `configure` target or model
// instance.
const AstNode* localContext(const LocalNamedReference* reference);

bool hasModifier(const VisibilityElement& node, std::string_view text);

// The fields a structure value of `type` sets by position: those of
// `getAllFields`.
std::vector<const Field*> valueFields(const AstNode* type);

// Whether `name`, in a call, is the return parameter of `operation`: its own
// name, or `return` when it has none.
bool namesReturnParameter(const Operation& operation, std::string_view name);

// The number of items of an Array, when its size is a non-negative constant.
std::optional<std::int64_t> arraySize(const Array& array);

// Reports why `value` does not fit `type`, looking inside arrays and
// structures. Nothing when either is null.
void checkValue(const Value* value, const Type* type,
                cursiv::ValidationAcceptor& accept);

// Reports on `node` a transfer from `output` to `input` whose fields are not of
// the same or a semantically equivalent type. Nothing when a path is unsafe or
// does not resolve.
void checkFieldConnection(const AstNode& node, const Path* output,
                          const Path* input,
                          cursiv::ValidationAcceptor& accept);

// Reports an event link whose source and sink are not of the same Event Type.
void checkEventConnection(const EventLink& link,
                          cursiv::ValidationAcceptor& accept);

// Whether `name` is an SMP identifier: a letter, then letters, digits and `_`.
bool validIdentifier(std::string_view name);

// Adds the default value of each parameter. One with no default stays unbound.
void bindParameters(Bindings& out,
                    const std::vector<TemplateParameter*>& parameters);

// The parameters of the instantiated assembly, then the arguments given to
// them.
void bindInstance(Bindings& out, const AssemblyInstance& instance);

// The template values at `node`, read from the root down so that an inner value
// replaces an outer one.
Bindings bindingsFor(const AstNode* node);

// `pattern` with each `{parameter}` replaced by its value. Nothing when one has
// no value or a brace is left open.
std::optional<std::string> expanded(std::string_view pattern,
                                    const Bindings& bindings);

// Every template parameter declared around `node`, with or without a value.
std::unordered_set<std::string_view> templateParameterNames(
    const AstNode& node);

// Reports each placeholder of `name` that no parameter declares, and an
// expanded name that is not a valid identifier. `context` tells the message who
// should declare it. Returns whether `name` is valid.
bool checkPattern(std::string_view name, const AstNode& node,
                  std::string_view context, cursiv::ValidationAcceptor& accept);

// `checkDuration` and `checkDate` report a `value` that is not valid. `label`
// names it in the message, and an empty value is accepted.
void checkDuration(const AstNode& node, std::string_view value,
                   std::string_view label, cursiv::ValidationAcceptor& accept);

void checkDate(const AstNode& node, std::string_view value,
               std::string_view label, cursiv::ValidationAcceptor& accept);

// Reports each `name = value` of a call whose name is no parameter of
// `operation`, whose value does not fit it, or that breaks the order of the
// parameters, and each parameter without a default that gets no value. Nothing
// for a null `operation`.
void checkOperationParameters(const AstNode& call, const Operation* operation,
                              const std::vector<ParameterValue*>& parameters,
                              cursiv::ValidationAcceptor& accept);

// What `path` has to end on, from the node that holds it.
PathRole roleOf(const Path& path);

// The role of a path read under `parent`. `first` says it is the path a link or
// a transfer starts from.
PathRole roleAt(const AstNode* parent, bool first);

// Whether a path of this role may end on `node`. `Input` and `Output` need a
// field with that modifier.
bool roleAccepts(PathRole role, const AstNode* node);

// How a message names `role`. `structure` says the field is one of a structure
// value.
std::string roleLabel(PathRole role, bool structure = false);

// The text of a named segment, placeholders included. Empty for `.` and `..`.
std::string_view segmentText(const PathSegment* segment);

// Whether the nearest explicit context, walking out through the enclosing
// configurations, is an assembly.
bool assemblyBacked(const ComponentConfiguration* configuration);

// Reports what is wrong with `path`: a `..`, a bad placeholder, a segment that
// names nothing or the wrong kind. Returns whether it is valid, which an
// `unsafe` path always is.
bool checkPath(const Path* path, cursiv::ValidationAcceptor& accept);

// Reports an interface link whose references do not fit the components at their
// other end. Nothing without an assembly to read it against.
void checkInterface(const InterfaceLink& link,
                    cursiv::ValidationAcceptor& accept);

// Reports the interface links of the document that connect a reference more
// times than its upper bound allows, over the whole tree of the assembly.
void checkReferenceBounds(const Assembly* assembly, const LinkBase* links,
                          cursiv::ValidationAcceptor& accept);

// What `segment` names, from its place in its path. Null outside a path.
const AstNode* resolveSegment(const PathSegment* segment);

const Type* expectedValueType(const Value* value);

// The name of `node` with its placeholders expanded, nothing if one has no
// value. A `context` that is a path or a segment gives the values along that
// path, otherwise those around `context`.
std::optional<std::string> concreteName(const NamedElement& node,
                                        const AstNode* context = nullptr);

// `members`, without the template, for the named elements only.
bool visitMembers(const AstNode* node,
                  cursiv::function_ref<bool(const NamedElement&)> visitor);

}  // namespace xsmp
