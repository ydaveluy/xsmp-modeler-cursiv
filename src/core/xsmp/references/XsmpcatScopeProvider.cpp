#include <algorithm>
#include <cursiv/workspace/Document.hpp>
#include <cursiv/workspace/Documents.hpp>
#include <cursiv/workspace/IndexManager.hpp>
#include <unordered_set>
#include <vector>
#include <xsmp/ast.hpp>
#include <xsmp/references/XsmpcatScopeProvider.hpp>
#include <xsmp/workspace/SourceUri.hpp>

namespace xsmp {

namespace {

using cursiv::AstNode;
using cursiv::AstNodeDescription;
using cursiv::ReferenceInfo;

// The document the reference is written in.
const cursiv::Document* home(const ReferenceInfo& context) {
  return context.document    ? context.document
         : context.container ? context.container->getDocument()
                             : nullptr;
}

// The field of a designated initializer, which only the structure being
// initialized can answer.
bool designated(const ReferenceInfo& context) {
  return context.property == "field"
         && cursiv::ast_cast<DesignatedInitializer>(context.container);
}

// The enclosing elements of a reference, and what they inherit, each once. A
// short list is searched, a long one remembered in a set.
class Scopes {
 public:
  Scopes() { _list.reserve(kShort); }

  [[nodiscard]] const std::vector<const AstNode*>& list() const noexcept {
    return _list;
  }

  void add(const AstNode* node) { _list.push_back(node); }

  // Adds `node`, then everything it inherits. A node already there stops a
  // cycle.
  void addWithBases(const AstNode* node) {
    if (!node || !isNew(node)) return;
    add(node);
    if (const auto* c = cursiv::ast_cast<Class>(node)) {
      if (c->base) addWithBases(c->base->get());
    } else if (const auto* c = cursiv::ast_cast<Component>(node)) {
      if (c->base) addWithBases(c->base->get());
      for (const auto& i : c->interface_) addWithBases(i.get());
    } else if (const auto* i = cursiv::ast_cast<Interface>(node)) {
      for (const auto& b : i->base) addWithBases(b.get());
    }
  }

 private:
  static constexpr std::size_t kShort = 32;

  [[nodiscard]] bool isNew(const AstNode* node) {
    if (_list.size() <= kShort)
      return std::ranges::find(_list, node) == _list.end();
    if (_seen.empty()) _seen.insert(_list.begin(), _list.end());
    return _seen.insert(node).second;
  }

  std::vector<const AstNode*> _list;
  std::unordered_set<const AstNode*> _seen;
};

}  // namespace

bool XsmpcatScopeProvider::acceptEntry(const ReferenceInfo& context,
                                       const AstNodeDescription& entry) const {
  const auto& reflection = *_xsmpServices.astReflection;
  return reflection.acceptsTag(reflection.referenceAccepts(context.reference),
                               entry.type)
         && reflection.isSubtype(
             entry.type, reflection.referenceNarrowTarget(context.reference,
                                                          context.narrow));
}

bool XsmpcatScopeProvider::visitLocal(
    const ReferenceInfo& context, std::string_view name,
    cursiv::function_ref<bool(const AstNodeDescription&)> visitor) const {
  if (!home(context)) return true;

  Scopes scopes;
  if (designated(context)) {
    const auto* expected =
        context.container && context.container->parent
            ? _xsmpServices.typeProvider->getType(*context.container->parent)
            : nullptr;
    if (cursiv::ast_cast<Structure>(expected)) scopes.addWithBases(expected);
  } else {
    // Resolving an owner's base must not consult that owner's inheritance.
    const auto* first = context.container;
    if (cursiv::ast_cast<Type>(first)) first = first->parent;
    for (const auto* scope = first; scope && scope != home(context);
         scope = scope->parent) {
      if (cursiv::ast_cast<Type>(scope))
        scopes.addWithBases(scope);
      else
        scopes.add(scope);
    }
  }

  const auto nameHash = cursiv::LocalSymbols::hash_name(name);
  for (const auto* scope : scopes.list()) {
    const auto* document = scope->getDocument();
    if (!document) continue;

    // Inside a type, a member may also be named through it: `Type.Member`, or
    // with the qualified name of the type.
    auto lookup = name;
    if (cursiv::ast_cast<const Type>(scope)
        && name.find('.') != std::string_view::npos) {
      const auto separator = name.rfind('.');
      const auto qualifier = name.substr(0, separator);
      const auto* type = cursiv::ast_cast<const Type>(scope);
      if (qualifier == type->name || hasQualifiedName(*type, qualifier))
        lookup = name.substr(separator + 1);
    }

    const auto& symbols = document->localSymbols;
    // The last declared of several homonyms wins.
    if (!lookup.empty()) {
      const AstNodeDescription* last = nullptr;
      const auto lookupHash = lookup.size() == name.size()
                                  ? nameHash
                                  : cursiv::LocalSymbols::hash_name(lookup);
      (void)symbols.find(scope, lookup, lookupHash)
          .for_each([&](const AstNodeDescription& entry) {
            if (entry.name == lookup && acceptEntry(context, entry))
              last = &entry;
            return true;
          });
      if (last && !visitor(*last)) return false;
      continue;
    }

    const auto entries = symbols.forContainer(scope);
    std::unordered_set<std::string_view> names;
    for (auto at = entries.end(); at != entries.begin();) {
      const auto& entry = *--at;
      if (!acceptEntry(context, entry) || !names.insert(entry.name).second)
        continue;
      if (!visitor(entry)) return false;
    }
  }
  return true;
}

const AstNodeDescription* XsmpcatScopeProvider::getScopeEntry(
    const ReferenceInfo& context, std::string_view name) const {
  const AstNodeDescription* found = nullptr;
  (void)visitLocal(context, name, [&](const AstNodeDescription& entry) {
    found = &entry;
    return false;
  });
  if (found || designated(context)) return found;

  // Looked for past this document: a later export may change the answer.
  if (const auto* document = home(context))
    if (auto* references = document->parseResult.references.get())
      references->importedNames.record(cursiv::LocalSymbols::hash_name(name));
  const auto& reflection = *_xsmpServices.astReflection;
  return getGlobalScopeEntry(context, name,
                             reflection.referenceAccepts(context.reference),
                             reflection, true);
}

bool XsmpcatScopeProvider::visitScopeEntries(
    const ReferenceInfo& context,
    cursiv::function_ref<bool(const AstNodeDescription&)> visitor) const {
  if (!visitLocal(context, {}, visitor)) return false;
  if (designated(context)) return true;
  const auto& reflection = *_xsmpServices.astReflection;
  return visitGlobalScopeEntries(context,
                                 reflection.referenceAccepts(context.reference),
                                 reflection, visitor);
}

bool XsmpcatScopeProvider::visitScopeEntries(
    const ReferenceInfo& context, std::string_view name,
    cursiv::function_ref<bool(const AstNodeDescription&)> visitor) const {
  if (name.empty()) return true;
  if (!visitLocal(context, name, visitor)) return false;
  if (designated(context)) return true;
  if (const auto* document = home(context))
    if (auto* references = document->parseResult.references.get())
      references->importedNames.record(cursiv::LocalSymbols::hash_name(name));
  const auto& reflection = *_xsmpServices.astReflection;
  return visitGlobalScopeEntries(context, name,
                                 reflection.referenceAccepts(context.reference),
                                 reflection, true, visitor);
}

const AstNodeDescription* XsmpcatScopeProvider::getGlobalScopeEntry(
    const ReferenceInfo& context, std::string_view name,
    cursiv::AcceptSet accepts, const cursiv::AstReflection& reflection,
    bool caseSensitive) const {
  // This document first, then user documents, then the built-ins. Among equals,
  // the last one visited.
  const AstNodeDescription* found = nullptr;
  auto priority = -1;
  const auto* document = home(context);
  (void)visitGlobalScopeEntries(
      context, name, accepts, reflection, caseSensitive,
      [&](const AstNodeDescription& entry) {
        const auto target =
            _xsmpServices.shared.documents->getDocument(entry.documentId);
        const auto candidate = document && entry.documentId == document->id ? 2
                               : target && !isBuiltinUri(target->uri)       ? 1
                                                                            : 0;
        if (candidate >= priority) {
          priority = candidate;
          found = &entry;
        }
        return true;
      });
  return found;
}

bool XsmpcatScopeProvider::visitGlobalScopeEntries(
    const ReferenceInfo& context, cursiv::AcceptSet,
    const cursiv::AstReflection&,
    cursiv::function_ref<bool(const AstNodeDescription&)> visitor) const {
  const auto* document = home(context);
  if (!document) return true;
  const auto visible =
      _xsmpServices.shared.projectManager->getVisibleDocumentIds(*document);
  const std::vector<cursiv::DocumentId> ids(visible->begin(), visible->end());
  return _xsmpServices.shared.indexManager->visitElements(
      nullptr, ids, [&](const AstNodeDescription& entry) {
        return !acceptEntry(context, entry) || visitor(entry);
      });
}

bool XsmpcatScopeProvider::visitGlobalScopeEntries(
    const ReferenceInfo& context, std::string_view name,
    cursiv::AcceptSet accepts, const cursiv::AstReflection& reflection,
    bool caseSensitive,
    cursiv::function_ref<bool(const AstNodeDescription&)> visitor) const {
  const auto* document = home(context);
  if (!document || name.empty()) return true;
  const auto visible =
      _xsmpServices.shared.projectManager->getVisibleDocumentIds(*document);
  const auto take = [&](const AstNodeDescription& entry) {
    return !visible->contains(entry.documentId) || !acceptEntry(context, entry)
           || visitor(entry);
  };
  auto& index = *_xsmpServices.shared.indexManager;
  if (!index.visitByName(name, caseSensitive, accepts, reflection, take))
    return false;

  // The built-in namespaces may be left out of a name.
  for (const auto prefix : {"Smp.", "Attributes."})
    if (!index.visitByName(std::string(prefix) + std::string(name),
                           caseSensitive, accepts, reflection, take))
      return false;
  return true;
}

}  // namespace xsmp
