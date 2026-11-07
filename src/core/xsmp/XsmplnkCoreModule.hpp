#pragma once

#include <cursiv/services/SharedCoreServices.hpp>
#include <memory>
#include <string>
#include <xsmp/XsmplnkCoreServices.hpp>

namespace xsmp {

// Applies the xsmplnk overrides (scoping, validation) on top of cursiv's
// default core services. The LSP path reuses it.
void installXsmplnkCoreModule(XsmplnkCoreServices& services);

// Builds the core-only xsmplnk language services.
std::unique_ptr<XsmplnkCoreServices> createXsmplnkServices(
    XsmpSharedCoreServices& sharedServices, std::string languageId = "xsmplnk");

// Registers the core-only xsmplnk services in `sharedServices`.
bool registerXsmplnkServices(cursiv::SharedCoreServices& sharedServices);

}  // namespace xsmp
