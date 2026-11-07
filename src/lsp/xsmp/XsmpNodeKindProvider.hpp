#pragma once

#include <cursiv/symbols/NodeKindProvider.hpp>

namespace xsmp {

// The icons of completion items and outline symbols in the XSMP languages, by
// AST type name, where Cursiv's default has one neutral kind. A type not listed
// shows as a field.
class XsmpNodeKindProvider final : public cursiv::NodeKindProvider {
 public:
  ::lsp::SymbolKind getSymbolKind(const cursiv::AstNode&) const override;
  ::lsp::SymbolKind getSymbolKind(
      const cursiv::AstNodeDescription&) const override;
  ::lsp::CompletionItemKind getCompletionItemKind(
      const cursiv::AstNode&) const override;
  ::lsp::CompletionItemKind getCompletionItemKind(
      const cursiv::AstNodeDescription&) const override;
};

}  // namespace xsmp
