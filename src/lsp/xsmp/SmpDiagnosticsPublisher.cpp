#include "SmpDiagnosticsPublisher.hpp"

#include <algorithm>
#include <cursiv/runtime/LanguageClient.hpp>
#include <xsmp/Json.hpp>
#include <xsmp/smp/SmpService.hpp>

namespace xsmp {

void SmpDiagnosticsPublisher::onBuildPhase(cursiv::DocumentState reached,
                                           std::span<cursiv::Document* const>,
                                           const cursiv::CancellationToken&) {
  if (reached != cursiv::DocumentState::Validated || !_shared.languageClient)
    return;

  // Taken in any case, so that the changes do not stay queued.
  if (const auto changes = takeMirrorChanges(_shared); _contentRefresh) {
    const auto refresh = [&](const std::string& uri) {
      _shared.languageClient->sendRequest(
          "workspace/textDocumentContent/refresh", JsonObject{{"uri", uri}},
          [](const Json&) {});
    };
    std::ranges::for_each(changes.changed, refresh);
    std::ranges::for_each(changes.deleted, refresh);
  }

  std::map<std::string, std::vector<Finding>> current;
  for (const auto& [uri, message] : sourceErrors(_shared))
    current[uri].emplace_back(message, 1);
  for (const auto& [uri, message] : sourceWarnings(_shared))
    current[uri].emplace_back(message, 2);

  // Only a URI whose findings changed is published again.
  for (const auto& [uri, findings] : current) {
    if (const auto old = _published.find(uri);
        old != _published.end() && old->second == findings)
      continue;
    ::lsp::PublishDiagnosticsParams params;
    params.uri = ::lsp::Uri::parse(uri);
    for (const auto& [message, severity] : findings) {
      ::lsp::Diagnostic diagnostic;
      // An import finding has no position in the XML file.
      diagnostic.range.start = {.line = 0, .character = 0};
      diagnostic.range.end = {.line = 0, .character = 0};
      diagnostic.message = message;
      diagnostic.severity = severity == 1 ? ::lsp::DiagnosticSeverity::Error
                                          : ::lsp::DiagnosticSeverity::Warning;
      diagnostic.source = "xsmp-smp";
      params.diagnostics.push_back(std::move(diagnostic));
    }
    _shared.languageClient->publishDiagnostics(std::move(params));
  }

  // A URI with no finding left gets an empty list, which clears it.
  for (const auto& [uri, findings] : _published)
    if (!current.contains(uri)) {
      ::lsp::PublishDiagnosticsParams params;
      params.uri = ::lsp::Uri::parse(uri);
      _shared.languageClient->publishDiagnostics(std::move(params));
    }

  _published = std::move(current);
}

}  // namespace xsmp
