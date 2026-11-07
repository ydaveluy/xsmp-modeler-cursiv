#pragma once

#include <cursiv/references/DefaultScopeComputation.hpp>

namespace xsmp {

// The editor view of an SMP XML file exports nothing while a native file stands
// beside it.
class DeploymentScopeComputation : public cursiv::DefaultScopeComputation {
 public:
  using DefaultScopeComputation::DefaultScopeComputation;
  std::vector<cursiv::AstNodeDescription> collectExportedSymbols(
      const cursiv::Document& document,
      const cursiv::CancellationToken& token) const override;
};

}  // namespace xsmp
