#include <memory>
#include <xsmp/DeploymentSemantics.hpp>
#include <xsmp/utils/XsmpUtils.hpp>

namespace xsmp {

// One Model Instance in the tree an assembly expands to, once each Assembly
// Instance is replaced by the assembly it names. The same model can occur
// several times, each with its own template bindings and absolute path.
struct AssemblyOccurrence {
  const Assembly* assembly = nullptr;
  const ModelInstance* model = nullptr;
  const Component* component = nullptr;
  Bindings bindings;
  std::string name;
  std::string absolutePath;
  std::vector<std::unique_ptr<AssemblyOccurrence>> children;
};

// The occurrence of `model` at `path`, and those below it. An Assembly Instance
// that would reopen an assembly on `stack`, the chain above it, is left out:
// `checkAssemblyInstance` reports that recursion.
std::unique_ptr<AssemblyOccurrence> createOccurrence(
    const Assembly* assembly, const ModelInstance* model, Bindings bindings,
    std::string path, std::unordered_set<const Assembly*> stack) {
  if (!assembly || !model) return {};
  auto occurrence = std::make_unique<AssemblyOccurrence>();
  occurrence->assembly = assembly;
  occurrence->model = model;
  occurrence->component = cursiv::ast_cast<Component>(targetType(model));
  occurrence->bindings = std::move(bindings);
  occurrence->absolutePath = std::move(path);

  for (auto* element : model->elements) {
    const auto* sub = cursiv::ast_cast<SubInstance>(element);
    if (!sub || !sub->instance) continue;
    const auto name = expanded(sub->instance->name, occurrence->bindings)
                          .value_or(std::string(sub->instance->name));
    const auto childPath = occurrence->absolutePath == "/"
                               ? "/" + name
                               : occurrence->absolutePath + "/" + name;

    std::unique_ptr<AssemblyOccurrence> child;
    if (const auto* instance = cursiv::ast_cast<ModelInstance>(sub->instance))
      child = createOccurrence(assembly, instance, occurrence->bindings,
                               childPath, stack);
    else if (const auto* instance =
                 cursiv::ast_cast<AssemblyInstance>(sub->instance)) {
      const auto* imported = instance->assembly.get();
      if (!imported || stack.contains(imported)) continue;
      // An imported assembly sees only the arguments of its instance, not the
      // bindings around it.
      Bindings arguments;
      bindInstance(arguments, *instance);
      auto childStack = stack;
      childStack.insert(imported);
      child = createOccurrence(imported, imported->model, std::move(arguments),
                               childPath, std::move(childStack));
    }

    if (child) {
      child->name = name;
      occurrence->children.push_back(std::move(child));
    }
  }
  return occurrence;
}

// The tree of `assembly` on its own, rooted at `/` with its parameter values.
std::unique_ptr<AssemblyOccurrence> createOccurrence(const Assembly* assembly) {
  if (!assembly) return {};
  Bindings bindings;
  bindParameters(bindings, assembly->parameters);
  return createOccurrence(assembly, assembly->model, std::move(bindings), "/",
                          {assembly});
}

// The named segments of `path`, head first, or nothing if it has an array
// index.
std::vector<const PathSegment*> namedSegments(const Path& path) {
  std::vector<const PathSegment*> result;
  if (path.head) result.push_back(path.head);
  for (auto* element : path.elements) {
    if (auto* member = cursiv::ast_cast<PathMember>(element))
      result.push_back(member->segment);
    else
      return {};  // Array elements are values, never component occurrences.
  }
  return result;
}

// The occurrence `path` leads to from `current`, or from `root` when absolute.
// `skipLast` stops before the last segment, which then names a member. Null
// when the path is unsafe or leaves the tree: `..`, an array index, a
// placeholder with no value, a child that does not exist.
const AssemblyOccurrence* resolveOccurrence(const Path* path,
                                            const AssemblyOccurrence* current,
                                            const AssemblyOccurrence* root,
                                            bool skipLast = false) {
  if (!path || !current || !root || path->unsafe) return nullptr;
  current = path->absolute ? root : current;

  auto segments = namedSegments(*path);
  if (!path->elements.empty() && segments.empty()) return nullptr;
  if (skipLast) {
    if (segments.empty()) return nullptr;
    segments.pop_back();
  }

  for (const auto* segment : segments) {
    if (cursiv::ast_cast<PathSelfSegment>(segment)) continue;
    if (cursiv::ast_cast<PathParentSegment>(segment)) return nullptr;
    const auto concrete = expanded(segmentText(segment), current->bindings);
    if (!concrete) return nullptr;
    const AssemblyOccurrence* next = nullptr;
    for (const auto& child : current->children)
      if (child->name == *concrete) {
        if (next)
          return nullptr;  // Ambiguous siblings are never chosen arbitrarily.
        next = child.get();
      }
    if (!next) return nullptr;
    current = next;
  }
  return current;
}

const AssemblyOccurrence* occurrenceForModel(
    const AssemblyOccurrence* occurrence, const ModelInstance* model) {
  if (!occurrence) return nullptr;
  if (occurrence->model == model) return occurrence;
  for (const auto& child : occurrence->children)
    if (const auto* found = occurrenceForModel(child.get(), model))
      return found;
  return nullptr;
}

// The occurrence a Component Link Base addresses: its name is a path from the
// occurrence of the enclosing block, or from the root.
const AssemblyOccurrence* occurrenceForBlock(const ComponentLinkBase* block,
                                             const AssemblyOccurrence* root) {
  if (!block || !root) return nullptr;
  const auto* parent = cursiv::ast_cast<ComponentLinkBase>(block->parent);
  return resolveOccurrence(
      block->name, parent ? occurrenceForBlock(parent, root) : root, root);
}

// The Reference called `name` on the occurrence's Component, inherited or not.
const Reference* referenceOf(const AssemblyOccurrence* occurrence,
                             std::string_view name) {
  if (!occurrence || !occurrence->component) return nullptr;
  const Reference* result = nullptr;
  members(occurrence->component, [&](const AstNode* node) {
    const auto* reference = cursiv::ast_cast<Reference>(node);
    if (!reference || reference->name != name) return true;
    result = reference;
    return false;
  });
  return result;
}

// The Reference an Interface Link's source path ends on. The last segment names
// it, and `owner` receives the occurrence the rest of the path reaches.
const Reference* sourceReference(const InterfaceLink& link,
                                 const AssemblyOccurrence* context,
                                 const AssemblyOccurrence* root,
                                 const AssemblyOccurrence*& owner) {
  if (!link.sourcePath || link.sourcePath->unsafe) return nullptr;
  const auto segments = namedSegments(*link.sourcePath);
  if (segments.empty()) return nullptr;
  owner = resolveOccurrence(link.sourcePath, context, root, true);
  if (!owner) return nullptr;
  const auto referenceName =
      expanded(segmentText(segments.back()), owner->bindings);
  return referenceName ? referenceOf(owner, *referenceName) : nullptr;
}

void checkInterface(const InterfaceLink& link,
                    cursiv::ValidationAcceptor& accept) {
  // The tree comes from the assembly around the link or, in a link base, from
  // the assembly it is anchored on.
  const auto* assembly = cursiv::container_of_type<Assembly>(&link);
  const auto* links = cursiv::container_of_type<LinkBase>(&link);
  auto root = createOccurrence(assembly ? assembly
                               : links  ? links->assembly.get()
                                        : nullptr);
  if (!root) return;

  const auto* context =
      assembly
          ? occurrenceForModel(root.get(),
                               cursiv::container_of_type<ModelInstance>(&link))
          : occurrenceForBlock(
                cursiv::container_of_type<ComponentLinkBase>(&link),
                root.get());

  if (!context) {
    if (link.backReference && !link.backReference->unsafe)
      accept
          .error(link,
                 "The selected reference shall resolve to a Reference of the "
                 "Client Component.")
          .property<&InterfaceLink::backReference>();
    return;
  }

  const auto* client = resolveOccurrence(link.clientPath, context, root.get());
  const AssemblyOccurrence* owner = nullptr;
  const auto* reference = sourceReference(link, context, root.get(), owner);

  if (reference && client && client->component && reference->interface_.get()
      && !isBaseOfReferenceType(reference->interface_.get(),
                                qualifiedName(*reference->interface_.get()),
                                client->component))
    accept
        .error(link,
               "The selected source reference shall be compatible with the "
               "Client Component.")
        .property<&InterfaceLink::sourcePath>();

  if (link.backReference && !link.backReference->unsafe && link.clientPath
      && !link.clientPath->unsafe) {
    const auto* back =
        referenceOf(client, link.backReference->reference.refText);
    if (!back)
      accept
          .error(link,
                 "The selected reference shall resolve to a Reference of the "
                 "Client Component.")
          .property<&InterfaceLink::backReference>();
    else if (owner && owner->component && back->interface_.get()
             && !isBaseOfReferenceType(back->interface_.get(),
                                       qualifiedName(*back->interface_.get()),
                                       owner->component))
      accept
          .error(link,
                 "The selected reference shall be compatible with the Owner "
                 "Component.")
          .property<&InterfaceLink::backReference>();
  }
}

// One link's use of a Reference. `source` says it is through the source path,
// rather than the back reference.
struct ReferenceUse {
  const InterfaceLink* link = nullptr;
  bool source = false;
};

// The links connected to one Reference of one occurrence, and its upper bound.
struct ReferenceUsages {
  std::int64_t upper = -1;
  std::string name;
  std::string path;
  std::vector<ReferenceUse> uses;
};

void checkReferenceBounds(const Assembly* assembly, const LinkBase* links,
                          cursiv::ValidationAcceptor& accept) {
  auto root = createOccurrence(assembly ? assembly
                               : links  ? links->assembly.get()
                                        : nullptr);
  if (!root) return;

  // Counted per occurrence: two instances of a model have separate connections.
  std::map<std::string, ReferenceUsages> buckets;
  const auto record = [&](const InterfaceLink& link,
                          const AssemblyOccurrence* context) {
    if (!context) return;
    const auto add = [&](const Reference* reference,
                         const AssemblyOccurrence* occurrence, bool source) {
      if (!reference || !occurrence) return;
      const auto upper = getUpper(*reference);
      if (!upper || *upper < 0) return;
      const auto key =
          occurrence->absolutePath + "::" + std::string(reference->name);
      auto& bucket = buckets[key];
      bucket.upper = *upper;
      bucket.name = reference->name;
      bucket.path = occurrence->absolutePath;
      bucket.uses.push_back({.link = &link, .source = source});
    };

    const AssemblyOccurrence* owner = nullptr;
    const auto* source = sourceReference(link, context, root.get(), owner);
    add(source, owner, true);
    if (link.backReference && !link.backReference->unsafe && link.clientPath
        && !link.clientPath->unsafe) {
      const auto* client =
          resolveOccurrence(link.clientPath, context, root.get());
      add(referenceOf(client, link.backReference->reference.refText), client,
          false);
    }
  };

  if (assembly) {
    const auto visit = [&](const auto& self,
                           const AssemblyOccurrence& occurrence) -> void {
      for (auto* node : occurrence.model->elements)
        if (auto* link = cursiv::ast_cast<InterfaceLink>(node))
          record(*link, &occurrence);
      for (const auto& child : occurrence.children) self(self, *child);
    };
    visit(visit, *root);
  } else if (links && links->getDocument()) {
    // A link base counts only its own links, not those of its assembly.
    for (auto* node : links->getDocument()->parseResult.nodes())
      if (auto* link = cursiv::ast_cast<InterfaceLink>(node);
          link && cursiv::container_of_type<LinkBase>(link) == links)
        record(*link, occurrenceForBlock(
                          cursiv::container_of_type<ComponentLinkBase>(link),
                          root.get()));
  }

  const auto* currentDocument =
      assembly ? assembly->getDocument() : links->getDocument();
  for (const auto& [key, bucket] : buckets) {
    if (bucket.uses.size() <= static_cast<std::uint64_t>(bucket.upper))
      continue;
    const auto message = "The Reference '" + bucket.name + "' of "
                         + (assembly ? "instance '" : "component path '")
                         + bucket.path + "' shall not be connected more than "
                         + std::to_string(bucket.upper) + " time(s).";
    for (const auto& use : bucket.uses) {
      // A link of an imported assembly counts, but cannot be reported from
      // here.
      if (use.link->getDocument() != currentDocument) continue;
      if (use.source)
        accept.error(*use.link, message).property<&InterfaceLink::sourcePath>();
      else
        accept.error(*use.link, message)
            .property<&InterfaceLink::backReference>();
    }
  }
}

}  // namespace xsmp
