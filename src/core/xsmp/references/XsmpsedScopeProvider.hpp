#pragma once

#include <xsmp/references/DeploymentScopeProvider.hpp>

namespace xsmp {

class XsmpsedScopeProvider final : public DeploymentScopeProvider {
 public:
  using DeploymentScopeProvider::DeploymentScopeProvider;

 protected:
  [[nodiscard]] std::optional<bool> visitOwnCandidates(
      const cursiv::ReferenceInfo& context, NodeVisitor give) const override;
};

}  // namespace xsmp
