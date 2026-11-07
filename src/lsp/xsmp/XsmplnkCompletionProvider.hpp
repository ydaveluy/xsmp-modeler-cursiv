#pragma once

#include <xsmp/DeploymentCompletionProvider.hpp>

namespace xsmp {

// Link base statements as snippets: a root component link base at the top of
// the file; in a block, a nested link base for each child path and each link
// its components allow.
class XsmplnkCompletionProvider final : public DeploymentCompletionProvider {
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
