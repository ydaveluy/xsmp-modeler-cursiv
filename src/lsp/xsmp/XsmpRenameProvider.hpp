#pragma once

#include <cursiv/navigation/DefaultRenameProvider.hpp>

namespace xsmp {

// Renaming in the XSMP languages. On top of the default, a reference written
// with a qualifier is rewritten with the same qualifier, also for the elements
// below a renamed namespace or type. An unresolved reference and a read-only
// document are refused.
class XsmpRenameProvider final : public cursiv::DefaultRenameProvider {
 public:
  using cursiv::DefaultRenameProvider::DefaultRenameProvider;
  std::optional<::lsp::Range> prepareRename(
      const cursiv::Document&, ::lsp::Position,
      const cursiv::CancellationToken& = {}) const override;

  std::optional<::lsp::WorkspaceEdit> rename(
      const cursiv::Document& document, ::lsp::Position position,
      std::string_view newName,
      const cursiv::CancellationToken& token = {}) const override;
};

}  // namespace xsmp
