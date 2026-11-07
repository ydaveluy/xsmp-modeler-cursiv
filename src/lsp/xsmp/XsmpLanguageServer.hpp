#pragma once

#include <cursiv/runtime/DefaultLanguageServer.hpp>
#include <mutex>
#include <xsmp/SharedLspServices.hpp>
#include <xsmp/SmpDiagnosticsPublisher.hpp>

namespace xsmp {

// The XSMP requests, and what workspace folders and watched files mean here.
class XsmpLanguageServer final : public cursiv::DefaultLanguageServer {
 public:
  explicit XsmpLanguageServer(XsmpSharedServices& shared);
  [[nodiscard]] ::lsp::InitializeResult initialize(
      const ::lsp::InitializeParams&) override;

  void didChangeWatchedFiles(
      const ::lsp::DidChangeWatchedFilesParams&) override;

  void didChangeWorkspaceFolders(
      const ::lsp::DidChangeWorkspaceFoldersParams&) override;

 private:
  void registerRequests();

  XsmpSharedServices& _shared;
  SmpDiagnosticsPublisher _diagnostics;

  // Requests run side by side; generation requests wait for each other here.
  mutable std::timed_mutex _generationMutex;
};

}  // namespace xsmp
