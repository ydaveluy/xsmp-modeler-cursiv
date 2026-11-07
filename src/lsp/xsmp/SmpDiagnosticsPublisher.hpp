#pragma once

#include <cursiv/services/SharedLspServices.hpp>
#include <cursiv/workspace/DocumentBuilder.hpp>
#include <map>

namespace xsmp {

// What the editor shows for SMP XML files, which no document carries: import
// errors and warnings as diagnostics on the physical file. Once a build has
// validated, it also asks the client to re-read the mirrors that changed.
class SmpDiagnosticsPublisher final : private cursiv::BuildPhaseListener {
 public:
  explicit SmpDiagnosticsPublisher(cursiv::SharedLspServices& shared)
      : _shared(shared) {
    _shared.documentBuilder->addBuildPhaseListener(*this);
  }

  ~SmpDiagnosticsPublisher() override {
    _shared.documentBuilder->removeBuildPhaseListener(*this);
  }

  // Whether the client refreshes virtual documents through the protocol.
  void useContentRefresh(bool enabled) noexcept { _contentRefresh = enabled; }

 private:
  void onBuildPhase(cursiv::DocumentState, std::span<cursiv::Document* const>,
                    const cursiv::CancellationToken&) override;

  cursiv::SharedLspServices& _shared;
  bool _contentRefresh = false;
  // What the client was last sent, by URI: a message and a severity, 1 for an
  // error and 2 for a warning.
  using Finding = std::pair<std::string, int>;
  std::map<std::string, std::vector<Finding>> _published;
};

}  // namespace xsmp
