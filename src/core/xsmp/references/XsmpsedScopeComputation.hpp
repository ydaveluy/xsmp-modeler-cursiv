#pragma once

#include <xsmp/references/DeploymentScopeComputation.hpp>

namespace xsmp {

class XsmpsedScopeComputation final : public DeploymentScopeComputation {
 public:
  using DeploymentScopeComputation::DeploymentScopeComputation;
  std::vector<cursiv::AstNodeDescription> collectExportedSymbols(
      const cursiv::Document& document,
      const cursiv::CancellationToken& token) const override;
};

}  // namespace xsmp
