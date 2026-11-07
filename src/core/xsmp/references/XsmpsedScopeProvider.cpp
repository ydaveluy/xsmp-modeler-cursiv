#include <xsmp/DeploymentSemantics.hpp>
#include <xsmp/references/XsmpsedScopeProvider.hpp>

namespace xsmp {

std::optional<bool> XsmpsedScopeProvider::visitOwnCandidates(
    const ReferenceInfo& context, NodeVisitor give) const {
  // A template argument names a parameter of the schedule of the executed task.
  const auto* argument = cursiv::ast_cast<TemplateArgument>(context.container);
  if (!argument) return std::nullopt;
  if (auto* execute = cursiv::ast_cast<ExecuteTask>(argument->parent))
    if (const auto* schedule =
            cursiv::container_of_type<Schedule>(execute->task.get()))
      for (auto* parameter : schedule->parameters)
        if (!give(parameter)) return false;
  return true;
}

}  // namespace xsmp
