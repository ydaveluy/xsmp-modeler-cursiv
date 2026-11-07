#pragma once

#include <cursiv/services/LspServices.hpp>
#include <cursiv/symbols/DocumentSymbolProvider.hpp>

namespace xsmp {

// The same details are used in document symbols and type hierarchy items.
std::optional<std::string> getXsmpNodeDetails(const cursiv::AstNode& node);

// The outline. Unlike the default, a statement with no name reads as it is
// written, such as `field link a -> b`; a symbol shows its type as its detail,
// and a deprecated one is marked.
class XsmpDocumentSymbolProvider final : public cursiv::DocumentSymbolProvider {
 public:
  explicit XsmpDocumentSymbolProvider(const cursiv::LspServices& services)
      : services(services) {}

  std::vector<::lsp::DocumentSymbol> getSymbols(
      const cursiv::Document&,
      const cursiv::CancellationToken& = {}) const override;

 private:
  const cursiv::LspServices& services;
};

}  // namespace xsmp
