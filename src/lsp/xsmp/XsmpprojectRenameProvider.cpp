#include <cursiv/references/References.hpp>
#include <cursiv/text/LspPosition.hpp>
#include <xsmp/XsmpprojectRenameProvider.hpp>
#include <xsmp/workspace/SourceUri.hpp>

namespace xsmp {

std::optional<::lsp::Range> XsmpprojectRenameProvider::prepareRename(
    const cursiv::Document& document, ::lsp::Position position,
    const cursiv::CancellationToken& token) const {
  if (isReadOnlyDocumentUri(document.uri)) return std::nullopt;
  const auto found = core().references->findDeclaration(
      document, document.textDocument().offsetAt(cursiv::from_lsp(position)),
      token);
  for (const auto* target : found.targets)
    if (const auto* home = target->getDocument();
        home && isReadOnlyDocumentUri(home->uri))
      return std::nullopt;

  auto range =
      cursiv::DefaultRenameProvider::prepareRename(document, position, token);
  if (range && found.span.end > found.span.begin + 1) {
    const auto text = document.textDocument().getText();
    if (text[found.span.begin] == '\'' || text[found.span.begin] == '"')
      range = cursiv::to_lsp(document.textDocument().rangeAt(
          found.span.begin + 1, found.span.end - 1));
  }
  return range;
}

std::optional<::lsp::WorkspaceEdit> XsmpprojectRenameProvider::rename(
    const cursiv::Document& document, ::lsp::Position position,
    std::string_view newName, const cursiv::CancellationToken& token) const {
  if (newName.empty()
      || newName.find_first_of("'\"\\\r\n") != std::string_view::npos
      || !prepareRename(document, position, token))
    return std::nullopt;
  return cursiv::DefaultRenameProvider::rename(
      document, position, "'" + std::string(newName) + "'", token);
}

}  // namespace xsmp
