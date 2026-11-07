#pragma once

#include <xsmp/DeploymentCompletionProvider.hpp>

namespace xsmp {

// Assembly statements as snippets: a root model instance at the top of the
// file; in a model instance, sub-instances (also one per container), field
// values and links; in a `configure` block, field values, operation calls,
// property values and global event subscriptions.
class XsmpasbCompletionProvider final : public DeploymentCompletionProvider {
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
