#pragma once

#include <cursiv/navigation/DefaultRenameProvider.hpp>

namespace xsmp {

// Renames a project, profile or tool. Their names are quoted strings, so the
// editor edits the text between the quotes and the new name is quoted back. A
// name written or declared in a read-only document cannot be renamed.
class XsmpprojectRenameProvider final : public cursiv::DefaultRenameProvider {
 public:
  using cursiv::DefaultRenameProvider::DefaultRenameProvider;
  std::optional<::lsp::Range> prepareRename(
      const cursiv::Document&, ::lsp::Position,
      const cursiv::CancellationToken& = {}) const override;

  std::optional<::lsp::WorkspaceEdit> rename(
      const cursiv::Document&, ::lsp::Position, std::string_view,
      const cursiv::CancellationToken& = {}) const override;
};

}  // namespace xsmp
