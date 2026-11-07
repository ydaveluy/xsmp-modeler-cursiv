#include <xsmp/DeploymentSemantics.hpp>
#include <xsmp/validation/XsmpcfgValidator.hpp>

namespace xsmp {

void XsmpcfgValidator::checkConfiguration(
    const Configuration& node, cursiv::ValidationAcceptor& accept) const {
  checkDocument(node, accept);
}

void XsmpcfgValidator::checkComponentConfiguration(
    const ComponentConfiguration& node,
    cursiv::ValidationAcceptor& accept) const {
  if (!node.name || !node.name->unsafe || !node.context.get<Component>())
    return;
  if (auto* parent = cursiv::ast_cast<ComponentConfiguration>(node.parent);
      parent && !assemblyBacked(parent))
    accept
        .warning(*node.name,
                 "The `unsafe` modifier is unnecessary when a nested Component "
                 "Configuration already declares an explicit Component "
                 "context.")
        .keyword("unsafe");
}

void XsmpcfgValidator::checkConfigurationUsage(
    const ConfigurationUsage& node, cursiv::ValidationAcceptor& accept) const {
  if (!node.path || !node.path->unsafe) return;
  if (auto* owner = cursiv::container_of_type<ComponentConfiguration>(&node);
      owner && !assemblyBacked(owner))
    accept
        .warning(*node.path,
                 "The `unsafe` modifier is unnecessary for a Configuration "
                 "Usage inside a Component-backed context.")
        .keyword("unsafe");
}

}  // namespace xsmp
