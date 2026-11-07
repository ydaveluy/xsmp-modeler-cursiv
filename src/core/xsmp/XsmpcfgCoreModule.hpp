#pragma once

#include <cursiv/services/SharedCoreServices.hpp>
#include <memory>
#include <string>
#include <xsmp/XsmpcfgCoreServices.hpp>

namespace xsmp {

// Applies the xsmpcfg overrides (scoping, validation) on top of cursiv's
// default core services. The LSP path reuses it.
void installXsmpcfgCoreModule(XsmpcfgCoreServices& services);

// Builds the core-only xsmpcfg language services.
std::unique_ptr<XsmpcfgCoreServices> createXsmpcfgServices(
    XsmpSharedCoreServices& sharedServices, std::string languageId = "xsmpcfg");

// Registers the core-only xsmpcfg services in `sharedServices`.
bool registerXsmpcfgServices(cursiv::SharedCoreServices& sharedServices);

}  // namespace xsmp
