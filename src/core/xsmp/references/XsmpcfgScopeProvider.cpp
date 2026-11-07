#include <xsmp/DeploymentSemantics.hpp>
#include <xsmp/references/XsmpcfgScopeProvider.hpp>

namespace xsmp {

bool XsmpcfgScopeProvider::allowsPathTarget(const Path* path,
                                            const AstNode* target,
                                            const PathSegment* segment) const {
  if (!path) return false;

  // A field path is not linked in a configuration with no context of its own
  // and no assembly around it.
  if (const auto* field = cursiv::ast_cast<FieldValue>(path->parent)) {
    const auto* configuration =
        cursiv::container_of_type<ComponentConfiguration>(field);
    if (configuration && !configuration->context.get<Component>()
        && !configuration->context.get<Assembly>()
        && !assemblyBacked(configuration))
      return false;
  }

  if (!DeploymentScopeProvider::allowsPathTarget(path, target, segment))
    return false;

  const auto* configuration =
      cursiv::ast_cast<ComponentConfiguration>(path->parent);
  const auto* usage = cursiv::ast_cast<ConfigurationUsage>(path->parent);
  const auto componentPath = (configuration && configuration->name == path)
                             || (usage && usage->path == path);
  if (!componentPath) return true;
  if (configuration
      && !cursiv::ast_cast<ComponentConfiguration>(configuration->parent))
    return false;  // A root name is a label; its explicit context applies to
                   // its children.
  // A catalogue container describes a possible occurrence. Configuration
  // names only reference actual model/assembly instances, not declarations.
  return cursiv::ast_cast<ModelInstance>(target)
         || cursiv::ast_cast<AssemblyInstance>(target);
}

std::optional<bool> XsmpcfgScopeProvider::visitOwnCandidates(
    const ReferenceInfo& context, NodeVisitor give) const {
  // The field designator of a structure value names a member of its type.
  const auto* segment =
      cursiv::ast_cast<ConcretePathNamedSegment>(context.container);
  const auto* field =
      segment && !segment->unsafe
          ? cursiv::ast_cast<CfgStructureFieldValue>(segment->parent)
          : nullptr;
  if (!field) return std::nullopt;
  return members(
      inferDeploymentValueType(cursiv::ast_cast<StructureValue>(field->parent)),
      give);
}

}  // namespace xsmp
