#include <cursiv/workspace/Document.hpp>
#include <xsmp/DeploymentSemantics.hpp>

namespace xsmp {

std::string_view nameOf(const AstNode* node) {
  return node && node->type && node->type->readName
             ? node->type->readName(*node)
             : std::string_view{};
}

const AstNodeDescription* descriptionOf(const AstNode* node) {
  if (!node) return nullptr;
  const auto* document = node->getDocument();
  if (!document) return nullptr;
  const auto& symbols = document->localSymbols;
  const auto name = nameOf(node);
  const AstNodeDescription* found = nullptr;
  (void)symbols.find(node->parent, name, cursiv::LocalSymbols::hash_name(name))
      .for_each([&](const AstNodeDescription& d) {
        if (d.node != node) return true;
        found = &d;
        return false;
      });
  if (found) return found;
  // An entry under another name than its own.
  for (const auto& d : symbols.forContainer(node->parent))
    if (d.node == node) return &d;
  return nullptr;
}

const AstNode* targetType(const AstNode* node) {
  if (auto* n = cursiv::ast_cast<ModelInstance>(node)) {
    if (auto* implementation = n->implementation.get()) return implementation;
    if (auto* sub = cursiv::ast_cast<SubInstance>(n->parent);
        sub && sub->container) {
      if (auto* container =
              cursiv::ast_cast<Container>(sub->container->reference.get())) {
        if (container->defaultComponent)
          return container->defaultComponent->get();
        return container->type.get();
      }
    }
    return nullptr;
  }

  if (auto* n = cursiv::ast_cast<AssemblyInstance>(node)) {
    auto* a = n->assembly.get();
    return a ? a->model : nullptr;
  }

  if (auto* n = cursiv::ast_cast<Assembly>(node)) return n->model;
  if (auto* n = cursiv::ast_cast<Field>(node)) return n->type.get();
  if (auto* n = cursiv::ast_cast<Property>(node)) return n->type.get();
  if (auto* n = cursiv::ast_cast<Container>(node)) return n->type.get();
  if (auto* n = cursiv::ast_cast<Reference>(node)) return n->interface_.get();
  if (auto* n = cursiv::ast_cast<Array>(node)) return n->itemType.get();
  if (auto* n = cursiv::ast_cast<ValueReference>(node)) return n->type.get();
  return node;
}

const AstNode* member(const AstNode* owner, std::string_view name,
                      const Bindings& bindings) {
  const AstNode* found = nullptr;
  const auto take = [&found](const AstNode* n) {
    found = n;
    return false;
  };
  // Only an instance has a name with placeholders.
  const auto instance = [&](const AstNode* n) {
    const auto written = nameOf(n);
    if (written == name) return take(n);
    if (written.find('{') == std::string_view::npos) return true;
    const auto concrete = expanded(written, bindings);
    return !concrete || *concrete != name || take(n);
  };
  const auto elements = [&](const AstNode& type, const auto& list) {
    return elementsNamed(type, list, name, take);
  };
  std::unordered_set<const AstNode*> visited;
  (void)membersImpl(owner, instance, elements, visited);
  return found;
}

std::string pathText(const Path* path) {
  if (!path) return {};
  const auto text = [](const PathSegment* segment) -> std::string_view {
    if (auto* concrete = cursiv::ast_cast<ConcretePathNamedSegment>(segment))
      return concrete->reference.refText;
    if (auto* pattern = cursiv::ast_cast<PatternPathNamedSegment>(segment))
      return pattern->text;
    if (cursiv::ast_cast<PathSelfSegment>(segment)) return ".";
    if (cursiv::ast_cast<PathParentSegment>(segment)) return "..";
    return {};
  };
  std::string result = path->absolute ? "/" : "";
  result += text(path->head);
  for (auto* element : path->elements) {
    if (auto* member = cursiv::ast_cast<PathMember>(element)) {
      result += member->separator;
      result += text(member->segment);
    } else if (auto* index = cursiv::ast_cast<PathIndex>(element))
      result += "[" + std::to_string(index->index) + "]";
  }
  return result;
}

// Where the climb to the context of `path` starts: the name of a configuration
// or of a link block is read from the level above it.
static const AstNode* contextStart(const Path* path) {
  const AstNode* from = path->parent;
  if (const auto* cfg = cursiv::ast_cast<ComponentConfiguration>(from);
      cfg && cfg->name == path)
    return cfg->parent;
  if (const auto* cfg = cursiv::ast_cast<AssemblyComponentConfiguration>(from);
      cfg && cfg->name == path)
    return cfg->parent;
  if (const auto* links = cursiv::ast_cast<ComponentLinkBase>(from);
      links && links->name == path)
    return links->parent;
  return from;
}

// A block named by its own path gives a context only as a component or an
// instance, and carries the values that path collected.
static PathResolution blockContext(const Path* name) {
  auto block = resolvePathWithBindings(name);
  if (!cursiv::ast_cast<ModelInstance>(block.node)
      && !cursiv::ast_cast<AssemblyInstance>(block.node)
      && !cursiv::ast_cast<Component>(block.node))
    block.node = nullptr;
  return block;
}

PathResolution pathContextResolution(const Path* path) {
  if (!path) return {};
  const auto at = [path](const AstNode* node) {
    return PathResolution{.node = node, .bindings = bindingsFor(path)};
  };

  // A field designator of a structure value is read in the structure's type.
  if (auto* field = cursiv::ast_cast<FieldValue>(path->parent))
    if (auto* structure = cursiv::ast_cast<StructureValue>(field->parent))
      return at(inferDeploymentValueType(structure));

  // An absolute path starts at the root of what its document is anchored on.
  if (path->absolute) {
    if (const auto* assembly = cursiv::container_of_type<Assembly>(path))
      return at(assembly->model);
    if (const auto* links = cursiv::container_of_type<LinkBase>(path);
        links && links->assembly.get())
      return at(links->assembly.get()->model);
    if (const auto* task = cursiv::container_of_type<Task>(path)) {
      if (auto* assembly = task->context.get<Assembly>())
        return at(assembly->model);
      if (auto* component = task->context.get<Component>())
        return at(component);
    }
    for (auto* n = path->parent; n; n = n->parent)
      if (auto* configuration = cursiv::ast_cast<ComponentConfiguration>(n)) {
        if (auto* assembly = configuration->context.get<Assembly>())
          return at(assembly->model);
        if (auto* component = configuration->context.get<Component>())
          return at(component);
      }
  }

  // Otherwise the nearest enclosing block says where the path is read.
  for (auto* n = contextStart(path); n; n = n->parent) {
    if (auto* cfg = cursiv::ast_cast<ComponentConfiguration>(n)) {
      if (auto* c = cfg->context.get<Component>()) return at(c);
      if (auto* a = cfg->context.get<Assembly>()) return at(a->model);
      return blockContext(cfg->name);
    }

    if (auto* cfg = cursiv::ast_cast<AssemblyComponentConfiguration>(n))
      return blockContext(cfg->name);

    if (auto* links = cursiv::ast_cast<ComponentLinkBase>(n))
      return blockContext(links->name);

    if (cursiv::ast_cast<ModelInstance>(n)
        || cursiv::ast_cast<AssemblyInstance>(n))
      return at(n);

    if (auto* task = cursiv::ast_cast<Task>(n)) {
      if (auto* c = task->context.get<Component>()) return at(c);
      if (auto* a = task->context.get<Assembly>()) return at(a->model);
      return at(nullptr);
    }

    if (auto* a = cursiv::ast_cast<Assembly>(n)) return at(a->model);
    if (auto* l = cursiv::ast_cast<LinkBase>(n)) {
      auto* a = l->assembly.get();
      return at(a ? a->model : nullptr);
    }
  }
  return at(nullptr);
}

const AstNode* pathContext(const Path* path) {
  return pathContextResolution(path).node;
}

PathResolution segmentResolution(const PathSegment* segment,
                                 const PathResolution& owner, bool first) {
  const AstNode* target = nullptr;
  if (auto* concrete = cursiv::ast_cast<ConcretePathNamedSegment>(segment)) {
    const auto name = concrete->reference.refText;
    target = member(owner.node, name, owner.bindings);
    // The head of a path may also name the current instance itself. An absolute
    // path starts there, and a relative one does when no member fits.
    const auto* path = cursiv::ast_cast<Path>(segment->parent);
    const auto ownerName = expanded(nameOf(owner.node), owner.bindings);
    if (first && path && owner.node
        && (nameOf(owner.node) == name || (ownerName && *ownerName == name))
        && (path->absolute || !target
            || (path->elements.empty() && !roleAccepts(roleOf(*path), target))))
      target = owner.node;
  } else if (cursiv::ast_cast<PathSelfSegment>(segment)) {
    return owner;
  } else if (cursiv::ast_cast<PathParentSegment>(segment)) {
    // `..` climbs from a sub-instance to the model that holds it.
    if (auto* instance = cursiv::ast_cast<NamedElement>(owner.node))
      if (auto* sub = cursiv::ast_cast<SubInstance>(instance->parent))
        target = cursiv::container_of_type<ModelInstance>(sub->parent);
  } else if (auto* pattern =
                 cursiv::ast_cast<PatternPathNamedSegment>(segment)) {
    const auto concrete = expanded(pattern->text, bindingsFor(pattern));
    if (concrete) target = member(owner.node, *concrete, owner.bindings);
  }

  PathResolution result{.node = target, .bindings = owner.bindings};
  if (auto* instance = cursiv::ast_cast<AssemblyInstance>(target))
    bindInstance(result.bindings, *instance);
  return result;
}

PathResolution resolvePathWithBindings(const Path* path) {
  if (!path) return {};
  auto current = pathContextResolution(path);
  if (path->head) current = segmentResolution(path->head, current, true);
  for (auto* element : path->elements) {
    if (auto* member = cursiv::ast_cast<PathMember>(element))
      current = segmentResolution(member->segment, current, false);
    else if (auto* index = cursiv::ast_cast<PathIndex>(element)) {
      // An index outside the array denotes nothing to type a value against.
      if (index->index < 0) return {};
      if (const auto* array = cursiv::ast_cast<Array>(targetType(current.node)))
        if (const auto size = arraySize(*array); size && index->index >= *size)
          return {};
      current.node = targetType(current.node);
    }
  }
  return current;
}

const AstNode* pathTarget(const Path* path) {
  return resolvePathWithBindings(path).node;
}

PathResolution ownerOfSegment(const PathSegment* wanted) {
  auto* path = cursiv::container_of_type<Path>(wanted);
  if (!path) return {};
  auto current = pathContextResolution(path);
  if (path->head == wanted || !path->head) return current;
  current = segmentResolution(path->head, current, true);
  for (auto* element : path->elements) {
    if (auto* member = cursiv::ast_cast<PathMember>(element)) {
      if (member->segment == wanted || member == wanted->parent) return current;
      current = segmentResolution(member->segment, current, false);
    } else if (cursiv::ast_cast<PathIndex>(element))
      current.node = targetType(current.node);
  }
  return {};
}

const AstNode* localContext(const LocalNamedReference* reference) {
  // The back reference of an interface link is a member of the client.
  if (auto* link = cursiv::ast_cast<InterfaceLink>(reference->parent))
    return pathTarget(link->clientPath);
  for (auto* n = reference->parent; n; n = n->parent) {
    if (auto* c = cursiv::ast_cast<AssemblyComponentConfiguration>(n))
      return pathTarget(c->name);
    if (cursiv::ast_cast<ModelInstance>(n)) return n;
  }
  return nullptr;
}

const AstNode* inferDeploymentValueType(const Value* value) {
  if (!value) return nullptr;
  const auto* parent = value->parent;
  if (auto* field = cursiv::ast_cast<FieldValue>(parent)) {
    if (field->field)
      return field->field->unsafe ? nullptr
                                  : targetType(pathTarget(field->field));
    return inferDeploymentValueType(field);
  }

  if (auto* field = cursiv::ast_cast<CfgStructureFieldValue>(parent))
    return field->field && !field->field->unsafe
               ? targetType(field->field->reference.get())
               : nullptr;

  if (auto* property = cursiv::ast_cast<PropertyValue>(parent))
    return property->property && !property->property->unsafe
               ? targetType(property->property->reference.get())
               : nullptr;

  if (auto* property = cursiv::ast_cast<SetProperty>(parent))
    return property->propertyPath && !property->propertyPath->unsafe
               ? targetType(pathTarget(property->propertyPath))
               : nullptr;

  if (auto* parameter = cursiv::ast_cast<ParameterValue>(parent)) {
    const Operation* operation = nullptr;
    if (auto* call = cursiv::ast_cast<OperationCall>(parameter->parent);
        call && call->operation && !call->operation->unsafe)
      operation = cursiv::ast_cast<Operation>(call->operation->reference.get());
    if (auto* call = cursiv::ast_cast<CallOperation>(parameter->parent);
        call && call->operationPath && !call->operationPath->unsafe)
      operation = cursiv::ast_cast<Operation>(pathTarget(call->operationPath));
    if (operation) {
      if (namesReturnParameter(*operation, parameter->parameter))
        return operation->returnParameter->type.get();
      for (auto* target : operation->parameter)
        if (target->name == parameter->parameter) return target->type.get();
    }
    return nullptr;
  }

  if (auto* array = cursiv::ast_cast<ArrayValue>(parent)) {
    auto* type = cursiv::ast_cast<Array>(inferDeploymentValueType(array));
    return type ? type->itemType.get() : nullptr;
  }

  if (auto* structure = cursiv::ast_cast<StructureValue>(parent)) {
    const auto fields = valueFields(inferDeploymentValueType(structure));
    std::unordered_set<const Field*> used;
    std::size_t next = 0;
    for (auto* element : structure->elements) {
      const Field* target = nullptr;
      if (auto* field = cursiv::ast_cast<CfgStructureFieldValue>(element);
          field && field->field)
        target = cursiv::ast_cast<Field>(field->field->reference.get());
      else if (auto* field = cursiv::ast_cast<FieldValue>(element);
               field && field->field) {
        target = cursiv::ast_cast<Field>(pathTarget(field->field));
      } else {
        // A value with no designator takes the next field not named so far.
        while (next < fields.size() && used.contains(fields[next])) ++next;
        if (next < fields.size()) target = fields[next++];
      }
      if (element == value) return target ? target->type.get() : nullptr;
      if (target) used.insert(target);
    }
  }

  return nullptr;
}

PathRole roleAt(const AstNode* parent, bool first) {
  if (cursiv::ast_cast<FieldValue>(parent)) return PathRole::Field;
  if (cursiv::ast_cast<ComponentConfiguration>(parent)
      || cursiv::ast_cast<AssemblyComponentConfiguration>(parent)
      || cursiv::ast_cast<ComponentLinkBase>(parent)
      || cursiv::ast_cast<ConfigurationUsage>(parent)
      || cursiv::ast_cast<ExecuteTask>(parent))
    return PathRole::Component;
  if (cursiv::ast_cast<FieldLink>(parent) || cursiv::ast_cast<Transfer>(parent))
    return first ? PathRole::Output : PathRole::Input;
  if (cursiv::ast_cast<EventLink>(parent))
    return first ? PathRole::Source : PathRole::Sink;
  if (cursiv::ast_cast<InterfaceLink>(parent))
    return first ? PathRole::Reference : PathRole::Component;
  if (cursiv::ast_cast<CallOperation>(parent)) return PathRole::Operation;
  if (cursiv::ast_cast<SetProperty>(parent)) return PathRole::Property;
  if (cursiv::ast_cast<Trigger>(parent)) return PathRole::EntryPoint;
  return PathRole::Any;
}

PathRole roleOf(const Path& path) {
  const auto* parent = path.parent;
  auto first = false;
  if (const auto* link = cursiv::ast_cast<InterfaceLink>(parent))
    first = link->sourcePath == &path;
  else if (const auto* other = cursiv::ast_cast<Link>(parent))
    first = other->ownerPath == &path;
  else if (const auto* transfer = cursiv::ast_cast<Transfer>(parent))
    first = transfer->outputFieldPath == &path;
  return roleAt(parent, first);
}

bool roleAccepts(PathRole role, const AstNode* node) {
  switch (role) {
    case PathRole::Component:
      return cursiv::ast_cast<ModelInstance>(node)
             || cursiv::ast_cast<AssemblyInstance>(node)
             || cursiv::ast_cast<Component>(node);
    case PathRole::Field:
      return cursiv::ast_cast<Field>(node);
    case PathRole::Input: {
      const auto* f = cursiv::ast_cast<Field>(node);
      return f && hasModifier(*f, "input");
    }
    case PathRole::Output: {
      const auto* f = cursiv::ast_cast<Field>(node);
      return f && hasModifier(*f, "output");
    }
    case PathRole::Source:
      return cursiv::ast_cast<EventSource>(node);
    case PathRole::Sink:
      return cursiv::ast_cast<EventSink>(node);
    case PathRole::Operation:
      return cursiv::ast_cast<Operation>(node);
    case PathRole::Property:
      return cursiv::ast_cast<Property>(node);
    case PathRole::EntryPoint:
      return cursiv::ast_cast<EntryPoint>(node);
    case PathRole::Reference:
      return cursiv::ast_cast<Reference>(node);
    default:
      return node != nullptr;
  }
}

}  // namespace xsmp

namespace xsmp {

const AstNode* resolveSegment(const PathSegment* segment) {
  const auto* path = cursiv::container_of_type<Path>(segment);
  return path ? segmentResolution(segment, ownerOfSegment(segment),
                                  path->head == segment)
                    .node
              : nullptr;
}

const Type* expectedValueType(const Value* value) {
  return cursiv::ast_cast<Type>(inferDeploymentValueType(value));
}

std::optional<std::string> concreteName(const NamedElement& node,
                                        const AstNode* context) {
  auto bindings = bindingsFor(context ? context : &node);
  if (const auto* segment = cursiv::ast_cast<ConcretePathNamedSegment>(context))
    bindings = ownerOfSegment(segment).bindings;
  else if (const auto* path = cursiv::ast_cast<Path>(context))
    bindings = resolvePathWithBindings(path).bindings;
  return expanded(node.name, bindings);
}

bool visitMembers(const AstNode* node,
                  cursiv::function_ref<bool(const NamedElement&)> visitor) {
  return members(node, [&](const AstNode* member) {
    const auto* named = cursiv::ast_cast<NamedElement>(member);
    return !named || visitor(*named);
  });
}

}  // namespace xsmp
