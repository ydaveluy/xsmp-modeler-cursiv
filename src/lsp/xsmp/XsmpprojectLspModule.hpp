#pragma once

#include <cursiv/services/SharedLspServices.hpp>
#include <memory>
#include <string>
#include <xsmp/XsmpprojectLspServices.hpp>

namespace xsmp {

/// Installs project completion, renaming, symbols and formatting on top of
/// the cursiv defaults.
void installXsmpprojectLspModule(XsmpprojectServices& services);

/// Builds the LSP-enabled `xsmp.project` language services.
std::unique_ptr<XsmpprojectServices> createXsmpprojectServices(
    XsmpSharedServices& sharedServices, std::string languageId = "xsmpproject");

/// Registers the LSP-enabled `xsmp.project` services in `sharedServices`.
bool registerXsmpprojectServices(cursiv::SharedLspServices& sharedServices);

}  // namespace xsmp
