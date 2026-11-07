#pragma once

#include <xsmp/references/DeploymentScopeProvider.hpp>

namespace xsmp {

class XsmpcfgScopeProvider final : public DeploymentScopeProvider {
 public:
  using DeploymentScopeProvider::DeploymentScopeProvider;

 protected:
  [[nodiscard]] bool allowsPathTarget(
      const Path* path, const cursiv::AstNode* target,
      const PathSegment* segment) const override;

  [[nodiscard]] std::optional<bool> visitOwnCandidates(
      const cursiv::ReferenceInfo& context, NodeVisitor give) const override;
};

}  // namespace xsmp
