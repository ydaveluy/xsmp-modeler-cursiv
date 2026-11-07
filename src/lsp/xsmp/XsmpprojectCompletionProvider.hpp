#pragma once

#include <cursiv/completion/DefaultCompletionProvider.hpp>
#include <set>
#include <string>
#include <string_view>
#include <xsmp/ast.hpp>

namespace xsmp {

class Registry;
class XsmpSharedCoreServices;

// A project statement is offered with what it can name: the profiles and tools
// not used yet, the projects it can depend on.
class XsmpprojectCompletionProvider final
    : public cursiv::DefaultCompletionProvider {
 public:
  using DefaultCompletionProvider::DefaultCompletionProvider;
  ::lsp::CompletionList getCompletion(
      const cursiv::Document& document, const ::lsp::CompletionParams& params,
      const cursiv::CancellationToken& token = {}) const override;

 protected:
  using Context = cursiv::CompletionContext;
  using Accept = cursiv::CompletionAcceptor;
  void completionFor(const Context&, const cursiv::NextFeature&,
                     Accept) const override;

  void completionForKeyword(const Context&, const cursiv::NextFeature&,
                            std::string_view keyword, Accept) const override;

  void visitReferenceCandidates(
      const cursiv::ReferenceInfo&, const Context&,
      cursiv::function_ref<bool(const cursiv::AstNodeDescription&)>)
      const override;

  [[nodiscard]] cursiv::CompletionValueItem createReferenceCompletionItem(
      const cursiv::AstNodeDescription&, const cursiv::ReferenceInfo&,
      const Context&) const override;

  [[nodiscard]] bool continueCompletion(
      const std::vector<::lsp::CompletionItem>&) const override {
    return true;
  }

 private:
  // `source`, then each `profile`, `tool` or `dependency` the project can still
  // name, or a snippet to fill in for a kind with no candidate.
  void completeStatement(const Context&, const Project&, Accept) const;
  void offerStandards(const Context&, Accept) const;
  [[nodiscard]] const XsmpSharedCoreServices* xsmpShared() const;

  // Null where there is none: completion then reads the workspace documents.
  [[nodiscard]] const Registry* registry() const;

  // The id a contribution goes by, through the registry or its aliases; `id`
  // itself where neither knows it.
  [[nodiscard]] std::string canonical(std::string_view id) const;

  // The id suggested for a kind of contribution; the kind itself if none.
  [[nodiscard]] std::string preferred(std::string_view kind) const;

  // The canonical ids of the profiles or tools the project already names.
  [[nodiscard]] std::set<std::string> used(const Project&,
                                           std::string_view kind) const;
};

}  // namespace xsmp
