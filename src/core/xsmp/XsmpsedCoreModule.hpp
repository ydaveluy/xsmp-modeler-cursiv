#pragma once

#include <cursiv/services/SharedCoreServices.hpp>
#include <memory>
#include <string>
#include <xsmp/XsmpsedCoreServices.hpp>

namespace xsmp {

// Applies the xsmpsed overrides (scoping, validation) on top of cursiv's
// default core services. The LSP path reuses it.
void installXsmpsedCoreModule(XsmpsedCoreServices& services);

// Builds the core-only xsmpsed language services.
std::unique_ptr<XsmpsedCoreServices> createXsmpsedServices(
    XsmpSharedCoreServices& sharedServices, std::string languageId = "xsmpsed");

// Registers the core-only xsmpsed services in `sharedServices`.
bool registerXsmpsedServices(cursiv::SharedCoreServices& sharedServices);

}  // namespace xsmp
