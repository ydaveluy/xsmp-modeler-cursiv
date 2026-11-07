#pragma once

#include <cursiv/completion/DefaultCompletionProvider.hpp>
#include <xsmp/CoreServices.hpp>

namespace xsmp {

// Catalogue completion. On top of the default, a keyword that starts a
// definition comes as a snippet that writes it, with a fresh `@uuid` for a
// type; candidates are filtered by what the model accepts at the site; and
// right after `=`, only values that fit the expected type are offered.
class XsmpcatCompletionProvider final
    : public cursiv::DefaultCompletionProvider {
 public:
  XsmpcatCompletionProvider(const cursiv::LspServices& lsp,
                            const XsmpcatCoreServices& core)
      : DefaultCompletionProvider(lsp), lsp(lsp), core(core) {}

  cursiv::CompletionProviderOptions completionOptions() const override {
    return {.triggerCharacters = {"=", "@"}, .allCommitCharacters = {}};
  }

  ::lsp::CompletionList getCompletion(
      const cursiv::Document&, const ::lsp::CompletionParams&,
      const cursiv::CancellationToken& = {}) const override;

 protected:
  void visitReferenceCandidates(
      const cursiv::ReferenceInfo&, const cursiv::CompletionContext&,
      cursiv::function_ref<bool(const cursiv::AstNodeDescription&)>)
      const override;

  [[nodiscard]] cursiv::CompletionValueItem createReferenceCompletionItem(
      const cursiv::AstNodeDescription&, const cursiv::ReferenceInfo&,
      const cursiv::CompletionContext&) const override;

  void completionFor(const cursiv::CompletionContext&,
                     const cursiv::NextFeature&,
                     cursiv::CompletionAcceptor) const override;

  [[nodiscard]] std::vector<::lsp::CompletionItem> deduplicateItems(
      std::vector<::lsp::CompletionItem>) const override;

  void completionForKeyword(const cursiv::CompletionContext&,
                            const cursiv::NextFeature&,
                            std::string_view keyword,
                            cursiv::CompletionAcceptor) const override;

  [[nodiscard]] bool continueCompletion(
      const std::vector<::lsp::CompletionItem>&) const override {
    return true;
  }

 private:
  const cursiv::LspServices& lsp;
  const XsmpcatCoreServices& core;

  // Whether a candidate may fill this member of this node, beyond what the
  // grammar can tell: visibility, no inheritance cycle, no duplicate in a list,
  // a fitting type. True where nothing forbids it or the node is unknown.
  bool acceptCandidate(const cursiv::AstNode*, std::string_view,
                       const cursiv::AstNodeDescription&) const;

  // A snippet item with no edit yet: the default adds it, and checks the label
  // against what is typed unless the item is marked `matched`.
  [[nodiscard]] cursiv::CompletionValueItem createSnippet(
      std::string label, std::string body, std::string detail) const;

  // A snippet choice `${index|a,b|}` of the visible types deriving from `type`,
  // those of this document first and sorted, or `${index:fallback}` if none. A
  // `Component` or `Interface` fallback also keeps to reference types.
  [[nodiscard]] std::string typeChoice(const cursiv::CompletionContext&,
                                       const cursiv::AstTypeTag* type,
                                       std::string_view fallback,
                                       unsigned index) const;
};

}  // namespace xsmp
