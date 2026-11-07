#pragma once

#include <xsmp/DeploymentCompletionProvider.hpp>

namespace xsmp {

// Schedule statements as snippets: tasks and events at the top of the file; in
// a task, calls, properties, entry point triggers, `execute` of the other
// tasks and field transfers.
class XsmpsedCompletionProvider final : public DeploymentCompletionProvider {
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
