#pragma once

#include <xsmp/DeploymentCompletionProvider.hpp>

namespace xsmp {

// Configuration statements as snippets: a root component configuration at the
// top of the file; in a nested block, a child configuration and field values;
// and `include`.
class XsmpcfgCompletionProvider final : public DeploymentCompletionProvider {
 public:
  using DeploymentCompletionProvider::DeploymentCompletionProvider;

 protected:
  [[nodiscard]] bool atStatement(const Context&) const override;
  void completeStatement(const Context&, const cursiv::AstNode* block,
                         Accept) const override;

  void completeKeyword(const Context&, const cursiv::AstNode* block,
                       std::string_view keyword, Accept) const override;
};

}  // namespace xsmp
