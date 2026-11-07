#include <xsmp/DeploymentSemantics.hpp>
#include <xsmp/references/XsmpasbScopeProvider.hpp>

namespace xsmp {

bool XsmpasbScopeProvider::allowsPathTarget(const Path* path,
                                            const AstNode* target,
                                            const PathSegment* segment) const {
  // The field designators of a structure value are not linked: the value check
  // reads them against the fields of the structure.
  if (path)
    if (const auto* field = cursiv::ast_cast<FieldValue>(path->parent);
        field && cursiv::ast_cast<StructureValue>(field->parent))
      return false;
  return DeploymentScopeProvider::allowsPathTarget(path, target, segment);
}

std::optional<bool> XsmpasbScopeProvider::visitOwnCandidates(
    const ReferenceInfo& context, NodeVisitor give) const {
  // A template argument names a parameter of the assembly it instantiates.
  const auto* argument = cursiv::ast_cast<TemplateArgument>(context.container);
  if (!argument) return std::nullopt;
  if (auto* instance = cursiv::ast_cast<AssemblyInstance>(argument->parent);
      instance && instance->assembly.get())
    for (auto* parameter : instance->assembly.get()->parameters)
      if (!give(parameter)) return false;
  return true;
}

}  // namespace xsmp
