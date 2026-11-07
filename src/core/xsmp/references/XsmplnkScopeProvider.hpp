#pragma once

#include <xsmp/references/DeploymentScopeProvider.hpp>

namespace xsmp {

class XsmplnkScopeProvider final : public DeploymentScopeProvider {
 public:
  using DeploymentScopeProvider::DeploymentScopeProvider;

 protected:
  [[nodiscard]] bool allowsPathTarget(
      const Path* path, const cursiv::AstNode* target,
      const PathSegment* segment) const override;
};

}  // namespace xsmp
