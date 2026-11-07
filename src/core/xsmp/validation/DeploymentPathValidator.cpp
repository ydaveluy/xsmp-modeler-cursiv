#include <xsmp/DeploymentSemantics.hpp>
#include <xsmp/utils/TreeUtils.hpp>

namespace xsmp {

std::string roleLabel(PathRole role, bool structure) {
  switch (role) {
    case PathRole::Component:
      return "a child Model Instance or Assembly Instance";
    case PathRole::Field:
      return structure ? "a Field of the current Structure value"
                       : "a Field of the current Component";
    case PathRole::Input:
      return "a Field marked as Input of the current Component";
    case PathRole::Output:
      return "a Field marked as Output of the current Component";
    case PathRole::Reference:
      return "a Reference of the current Component";
    default:
      return "a supported member of the current Component";
  }
}

std::string_view segmentText(const PathSegment* segment) {
  if (auto* concrete = cursiv::ast_cast<ConcretePathNamedSegment>(segment))
    return concrete->reference.refText;
  if (auto* pattern = cursiv::ast_cast<PatternPathNamedSegment>(segment))
    return pattern->text;
  return {};
}

bool assemblyBacked(const ComponentConfiguration* configuration) {
  for (const auto* current = configuration; current;
       current = cursiv::ast_cast<ComponentConfiguration>(current->parent)) {
    if (current->context.get<Assembly>()) return true;
    if (current->context.get<Component>()) return false;
  }
  return false;
}

// Who declares the placeholders of a schedule's path: its task may inherit an
// Assembly's. Null outside a schedule.
const char* schedulePlaceholderContext(const Path* path) {
  const auto* schedule = cursiv::container_of_type<Schedule>(path);
  if (!schedule) return nullptr;
  const auto* task = cursiv::container_of_type<Task>(path);
  if (!task || !task->context.get<Assembly>()) return "enclosing Schedule";
  return schedule->parameters.empty()
             ? "inherited Assembly context"
             : "enclosing Schedule or inherited Assembly context";
}

bool checkPath(const Path* path, cursiv::ValidationAcceptor& accept) {
  // An `unsafe` path asks not to be checked.
  if (!path || path->unsafe) return true;
  auto resolution = pathContextResolution(path);
  const auto* current = resolution.node;
  const auto role = roleOf(*path);

  // A usage inside a Component-backed configuration is taken as written.
  if (cursiv::ast_cast<ConfigurationUsage>(path->parent))
    if (const auto* owner =
            cursiv::container_of_type<ComponentConfiguration>(path);
        owner && !assemblyBacked(owner))
      return true;

  std::vector<const PathSegment*> segments;
  if (path->head) segments.push_back(path->head);
  for (auto* element : path->elements)
    if (auto* member = cursiv::ast_cast<PathMember>(element))
      segments.push_back(member->segment);

  // Naming a Component Configuration adds rules on its explicit context.
  const auto* configuration =
      cursiv::ast_cast<ComponentConfiguration>(path->parent);
  if (configuration) {
    auto* parent =
        cursiv::ast_cast<ComponentConfiguration>(configuration->parent);
    const auto hasContext = configuration->context.get<Component>()
                            || configuration->context.get<Assembly>();
    // An explicit context settles the path, but its placeholders still count.
    if ((configuration->context.get<Component>()
         && (!parent || !assemblyBacked(parent)))
        || (configuration->context.get<Assembly>() && !parent)) {
      auto valid = true;
      for (const auto* segment : segments)
        valid = checkPattern(segmentText(segment), *segment,
                             "enclosing Assembly context", accept)
                && valid;
      return valid;
    }
    if (parent && current && !assemblyBacked(parent) && !hasContext) {
      accept
          .error(*configuration,
                 "A safe Component Configuration inside a Component-backed "
                 "context shall declare an explicit context.")
          .property<&ComponentConfiguration::name>();
      return false;
    }
    if (parent && assemblyBacked(parent) && hasContext) {
      accept
          .error(*configuration,
                 "A safe Component Configuration inside an Assembly-backed "
                 "context shall not declare an explicit context.")
          .property<&ComponentConfiguration::context>();
      return false;
    }
  }

  auto patternsValid = true;
  // What declares this path's placeholders, as the diagnostics call it.
  const std::string_view patternContext =
      schedulePlaceholderContext(path) ? schedulePlaceholderContext(path)
      : cursiv::container_of_type<LinkBase>(path) ? "anchored Assembly"
      : cursiv::container_of_type<Assembly>(path)
          ? "enclosing Assembly"
          : "enclosing Assembly context";

  for (const auto* segment : segments) {
    if (cursiv::ast_cast<PathParentSegment>(segment)) {
      const auto elementsRange =
          (cursiv::container_of_type<LinkBase>(path)
           && !cursiv::ast_cast<ComponentLinkBase>(path->parent)
           && !cursiv::ast_cast<InterfaceLink>(path->parent))
          || (cursiv::container_of_type<Schedule>(path)
              && !cursiv::ast_cast<ExecuteTask>(path->parent));
      accept.error(*path, "Paths shall not contain '..'.")
          .range(rangeForFeature(*path, elementsRange ? "elements" : ""));
      return false;
    }

    // A link base with no Assembly anchor is reported once, by `checkLinkBase`.
    const auto* linkBase = cursiv::container_of_type<LinkBase>(path);
    if (!linkBase || !linkBase->assembly.refText.empty())
      patternsValid =
          checkPattern(segmentText(segment), *segment, patternContext, accept)
          && patternsValid;
  }

  // What the path reads in is reported on its own when it does not resolve.
  if (!patternsValid || !targetType(current)) return patternsValid;

  if (path->absolute
      && cursiv::ast_cast<AssemblyComponentConfiguration>(path->parent)) {
    accept.error(*path, "InstancePath shall be relative.");
    return false;
  }
  if (path->absolute && cursiv::container_of_type<Assembly>(path)
      && cursiv::ast_cast<FieldValue>(path->parent)) {
    accept.error(*path,
                 "Field paths in an Assembly shall be relative to the current "
                 "component instance.");
    return false;
  }

  // Reports that `segment`, or the whole path when null, is not what `wanted`
  // asks for. Always false, so the caller can return it.
  const auto bad = [&](const PathSegment* segment, PathRole wanted,
                       bool structure = false) {
    const auto text = segmentText(segment);
    if (text.empty())
      accept.error(*path,
                   "The path shall resolve to a supported member of the "
                   "current Component.");
    else
      accept.error(
          *segment,
          "The path segment '" + std::string(text) + "' shall resolve to "
              + (wanted == PathRole::Field && !structure
                         && (cursiv::container_of_type<
                                 AssemblyComponentConfiguration>(path)
                             || cursiv::container_of_type<
                                 ComponentConfiguration>(path))
                     ? std::string("a Field of the configured Component")
                     : roleLabel(wanted, structure))
              + ".");
    return false;
  };

  // Steps `current` through one named segment. A segment before the last shall
  // lead to an instance or a Field, and the last shall fit the role of the
  // path.
  const auto advance = [&](const PathSegment* segment, bool first, bool last) {
    const bool structure = cursiv::ast_cast<Structure>(targetType(current));
    const auto nextResolution = segmentResolution(segment, resolution, first);
    const auto* next = nextResolution.node;
    if (!next)
      return bad(segment,
                 !last
                         && (cursiv::ast_cast<ModelInstance>(current)
                             || cursiv::ast_cast<AssemblyInstance>(current))
                     ? PathRole::Component
                     : role,
                 structure);
    if (!last && !cursiv::ast_cast<ModelInstance>(next)
        && !cursiv::ast_cast<AssemblyInstance>(next)
        && !cursiv::ast_cast<Field>(next))
      return bad(segment,
                 cursiv::ast_cast<ModelInstance>(current)
                         || cursiv::ast_cast<AssemblyInstance>(current)
                     ? PathRole::Component
                     : role,
                 structure);
    if (last && !roleAccepts(role, next)) return bad(segment, role, structure);
    current = next;
    resolution = nextResolution;
    return true;
  };

  if (path->head && !advance(path->head, true, path->elements.empty()))
    return false;
  // With no head the path names its context, which fits only these two roles.
  if (!path->head && role != PathRole::Component && role != PathRole::Any)
    return bad(nullptr, role);

  for (std::size_t index = 0; index < path->elements.size(); ++index) {
    const auto* element = path->elements[index];
    if (auto* member = cursiv::ast_cast<PathMember>(element)) {
      if (!advance(member->segment, false, index + 1 == path->elements.size()))
        return false;
    } else if (auto* subscript = cursiv::ast_cast<PathIndex>(element)) {
      const auto* array = cursiv::ast_cast<Array>(targetType(current));
      if (!array) {
        accept.error(*subscript,
                     "An array index shall only be applied to an Array value.");
        return false;
      }
      if (subscript->index < 0) {
        accept.error(*subscript, "Array index shall not be negative.");
        return false;
      }
      if (const auto size = arraySize(*array);
          size && subscript->index >= *size) {
        accept.error(*subscript,
                     "Array index " + std::to_string(subscript->index)
                         + " is out of bounds for the array of size "
                         + std::to_string(*size) + ".");
        return false;
      }
      current = array->itemType.get();
      resolution.node = current;
    }
  }
  return true;
}

}  // namespace xsmp
