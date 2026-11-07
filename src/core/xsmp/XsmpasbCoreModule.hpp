#pragma once

#include <cursiv/services/SharedCoreServices.hpp>
#include <memory>
#include <string>
#include <xsmp/XsmpasbCoreServices.hpp>

namespace xsmp {

// Applies the xsmpasb overrides (scoping, validation) on top of cursiv's
// default core services. The LSP path reuses it.
void installXsmpasbCoreModule(XsmpasbCoreServices& services);

// Builds the core-only xsmpasb language services.
std::unique_ptr<XsmpasbCoreServices> createXsmpasbServices(
    XsmpSharedCoreServices& sharedServices, std::string languageId = "xsmpasb");

// Registers the core-only xsmpasb services in `sharedServices`.
bool registerXsmpasbServices(cursiv::SharedCoreServices& sharedServices);

}  // namespace xsmp
