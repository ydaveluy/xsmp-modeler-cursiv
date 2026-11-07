#pragma once

#include <cursiv/references/DefaultScopeComputation.hpp>

namespace xsmp {

class XsmpprojectScopeComputation final
    : public cursiv::DefaultScopeComputation {
 public:
  using DefaultScopeComputation::DefaultScopeComputation;
  std::vector<cursiv::AstNodeDescription> collectExportedSymbols(
      const cursiv::Document& document,
      const cursiv::CancellationToken& token) const override;
};

}  // namespace xsmp
