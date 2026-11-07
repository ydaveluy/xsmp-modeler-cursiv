#include <cursiv/workspace/Document.hpp>
#include <xsmp/DeploymentSemantics.hpp>
#include <xsmp/references/DeploymentScopeComputation.hpp>
#include <xsmp/workspace/SourceUri.hpp>

namespace xsmp {

std::vector<AstNodeDescription>
DeploymentScopeComputation::collectExportedSymbols(
    const cursiv::Document& document,
    const cursiv::CancellationToken& token) const {
  if (hasNativeSidecar(document.uri, services.shared())) return {};
  return DefaultScopeComputation::collectExportedSymbols(document, token);
}

}  // namespace xsmp
