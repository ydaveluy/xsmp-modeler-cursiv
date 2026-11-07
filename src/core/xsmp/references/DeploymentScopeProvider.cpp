#include <cursiv/syntax-tree/AstReflection.hpp>
#include <cursiv/workspace/Document.hpp>
#include <cursiv/workspace/IndexManager.hpp>
#include <xsmp/DeploymentSemantics.hpp>
#include <xsmp/SharedCoreServices.hpp>
#include <xsmp/references/DeploymentScopeProvider.hpp>

namespace xsmp {

const AstNodeDescription* DeploymentScopeProvider::getScopeEntry(
    const ReferenceInfo& context, std::string_view name) const {
  // An `unsafe` name is not linked.
  if (auto* segment =
          cursiv::ast_cast<ConcretePathNamedSegment>(context.container);
      segment && segment->unsafe)
    return nullptr;
  if (auto* local = cursiv::ast_cast<LocalNamedReference>(context.container);
      local && local->unsafe)
    return nullptr;

  // A path segment resolves by itself among the members of its owner: `name`
  // plays no part.
  if (auto* segment =
          cursiv::ast_cast<ConcretePathNamedSegment>(context.container);
      segment && cursiv::container_of_type<Path>(segment)) {
    const auto owner = ownerOfSegment(segment);
    const auto selected = segmentResolution(
        segment, owner,
        cursiv::container_of_type<Path>(segment)
            && cursiv::container_of_type<Path>(segment)->head == segment);
    if (!allowsPathTarget(cursiv::container_of_type<Path>(segment),
                          selected.node, segment))
      return nullptr;
    const auto* entry = descriptionOf(selected.node);
    return entry && accepts(context, *entry) ? entry : nullptr;
  }

  const AstNodeDescription* result = nullptr;
  if (name.empty()) return result;
  visitCandidates(context, name, [&](const AstNodeDescription& d) {
    result = &d;
    return false;
  });
  return result;
}

bool DeploymentScopeProvider::visitScopeEntries(
    const ReferenceInfo& context,
    cursiv::function_ref<bool(const AstNodeDescription&)> visitor) const {
  return visitCandidates(context, {}, visitor);
}

bool DeploymentScopeProvider::visitScopeEntries(
    const ReferenceInfo& context, std::string_view name,
    cursiv::function_ref<bool(const AstNodeDescription&)> visitor) const {
  return name.empty() || visitCandidates(context, name, visitor);
}

bool DeploymentScopeProvider::allowsPathTarget(
    const Path* path, const AstNode* target, const PathSegment* segment) const {
  if (!path) return false;
  const auto* last = path->head;
  for (const auto* element : path->elements)
    if (const auto* member = cursiv::ast_cast<PathMember>(element))
      last = member->segment;

  // Only where the path ends does the kind of target matter.
  return segment != last || roleAccepts(roleOf(*path), target);
}

bool DeploymentScopeProvider::accepts(const ReferenceInfo& context,
                                      const AstNodeDescription& d) const {
  if (!d.type) return false;
  const auto& reflection = *services.astReflection;
  if (!reflection.isSubtype(d.type, reflection.referenceNarrowTarget(
                                        context.reference, context.narrow)))
    return false;
  const auto targets = reflection.referenceTargets(context.reference);
  if (targets.empty()) return true;
  for (auto target : targets) {
    const auto* tag = reflection.typeTagOf(target);
    if (tag && d.type->derivesFrom(tag)) return true;
  }
  return false;
}

bool DeploymentScopeProvider::visitCandidates(
    const ReferenceInfo& context, std::string_view name,
    cursiv::function_ref<bool(const AstNodeDescription&)> visitor) const {
  const auto giveNode = [&](const AstNode* node) {
    if (!name.empty() && nameOf(node) != name) return true;
    if (const auto* segment =
            cursiv::ast_cast<ConcretePathNamedSegment>(context.container);
        segment && cursiv::container_of_type<Path>(segment)
        && !allowsPathTarget(cursiv::container_of_type<Path>(segment), node,
                             segment))
      return true;
    auto* d = descriptionOf(node);
    return !d || !accepts(context, *d) || visitor(*d);
  };
  // Those of a node in the order of `members`, with or without a name.
  const auto giveMembers = [&](const AstNode* node) {
    return name.empty() ? members(node, giveNode)
                        : membersNamed(node, name, giveNode);
  };

  if (const auto own = visitOwnCandidates(context, giveNode)) return *own;

  if (auto* segment =
          cursiv::ast_cast<ConcretePathNamedSegment>(context.container)) {
    if (segment->unsafe) return true;
    auto* path = cursiv::container_of_type<Path>(segment);
    const auto owner = ownerOfSegment(segment);
    if (path && path->head == segment && owner.node && !giveNode(owner.node))
      return false;
    return giveMembers(owner.node);
  }

  if (auto* local = cursiv::ast_cast<LocalNamedReference>(context.container))
    return local->unsafe || giveMembers(localContext(local));

  // Any other reference: the enclosing scopes, then the visible documents.
  const auto* doc =
      context.document
          ? context.document
          : (context.container ? context.container->getDocument() : nullptr);
  const auto give = [&](const AstNodeDescription& d) {
    return !accepts(context, d) || visitor(d);
  };
  const auto hash = cursiv::LocalSymbols::hash_name(name);
  for (const auto* scope = context.container; scope && doc && scope != doc;
       scope = scope->parent) {
    if (name.empty()) {
      for (const auto& d : doc->localSymbols.forContainer(scope))
        if (!give(d)) return false;
    } else if (!doc->localSymbols.find(scope, name, hash)
                    .for_each([&](const AstNodeDescription& d) {
                      return d.name != name || give(d);
                    })) {
      return false;
    }
  }
  std::shared_ptr<const std::unordered_set<cursiv::DocumentId>> visible;
  if (doc)
    if (auto* shared =
            dynamic_cast<const XsmpSharedCoreServices*>(&services.shared());
        shared && shared->projectManager)
      visible = shared->projectManager->getVisibleDocumentIds(*doc);
  auto& index = *services.shared().indexManager;
  if (name.empty()) {
    std::vector<cursiv::DocumentId> ids;
    if (visible) ids.assign(visible->begin(), visible->end());
    return index.visitElements(nullptr, ids, give);
  }
  return index.visitByName(
      name, true, {}, *services.astReflection,
      [&](const AstNodeDescription& d) {
        return (visible && !visible->contains(d.documentId)) || give(d);
      });
}

}  // namespace xsmp
