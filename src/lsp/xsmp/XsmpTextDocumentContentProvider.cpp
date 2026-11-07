#include <cursiv/utils/UriUtils.hpp>
#include <cursiv/workspace/Documents.hpp>
#include <cursiv/workspace/WorkspaceManager.hpp>
#include <filesystem>
#include <xsmp/XsmpTextDocumentContentProvider.hpp>
#include <xsmp/smp/SmpService.hpp>
#include <xsmp/workspace/SourceUri.hpp>

namespace xsmp {

namespace {

// The text of a document only the server holds: a library, or the XSMP form of
// an SMP file.
std::optional<std::string> serverFileContent(
    const cursiv::SharedCoreServices& shared, std::string_view spelled) {
  const auto uri = isReadOnlyDocumentUri(spelled) ? canonicalVirtualUri(spelled)
                                                  : std::string(spelled);
  const auto mirror = isSmpMirrorUri(uri);
  // A mirror is gone once a native file stands beside its XML file, and so is
  // one the workspace scan leaves out.
  if (mirror
      && (hasNativeSidecar(uri, shared)
          || !shared.workspaceManager->isIncluded(
              {.isFile = true, .isDirectory = false, .uri = uri})))
    return std::nullopt;
  if (const auto document = shared.documents->getDocument(uri))
    return std::string(document->textDocument().getText());

  // A mirror not in the workspace is rendered from its XML file.
  if (!mirror) return std::nullopt;
  const auto source = cursiv::file_uri_to_path(sourceDocumentUri(uri));
  if (!source) return std::nullopt;
  ImportOptions options;
  for (const auto& folder : shared.workspaceManager->workspaceFolders())
    if (const auto root = cursiv::file_uri_to_path(folder.uri))
      options.searchRoots.emplace_back(*root);

  try {
    return renderImportedDocument(*source, options).content;
  } catch (const std::exception&) {
    return std::nullopt;
  }
}

}  // namespace

std::optional<::lsp::TextDocumentContentResult>
XsmpTextDocumentContentProvider::provideTextDocumentContent(
    const ::lsp::TextDocumentContentParams& params) const {
  auto content = serverFileContent(shared, params.uri.toString());
  if (!content) return std::nullopt;
  return ::lsp::TextDocumentContentResult{std::move(*content)};
}

}  // namespace xsmp
