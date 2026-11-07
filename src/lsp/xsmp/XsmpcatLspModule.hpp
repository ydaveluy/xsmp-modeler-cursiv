#pragma once

#include <cursiv/services/SharedLspServices.hpp>
#include <memory>
#include <string>
#include <xsmp/XsmpcatLspServices.hpp>

namespace xsmp {

/// Installs the catalogue editor providers on its typed language container.
void installXsmpcatLspModule(XsmpcatServices& services);

/// Builds the LSP-enabled xsmpcat language services.
std::unique_ptr<XsmpcatServices> createXsmpcatServices(
    XsmpSharedServices& sharedServices, std::string languageId = "xsmpcat");

/// Registers the LSP-enabled Xsmpcat services in `sharedServices`.
bool registerXsmpcatServices(cursiv::SharedLspServices& sharedServices);

}  // namespace xsmp
