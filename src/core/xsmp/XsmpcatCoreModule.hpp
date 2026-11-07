#pragma once

#include <cursiv/services/SharedCoreServices.hpp>
#include <memory>
#include <string>
#include <xsmp/XsmpcatCoreServices.hpp>

namespace xsmp {

/// Applies the xsmpcat core overrides (parser, qualified names, scoping) on top
/// of cursiv's default core services. The LSP path reuses it.
void installXsmpcatCoreModule(XsmpcatCoreServices& services);

/// Builds the core-only xsmpcat language services.
std::unique_ptr<XsmpcatCoreServices> createXsmpcatServices(
    XsmpSharedCoreServices& sharedServices, std::string languageId = "xsmpcat");

/// Registers the core-only xsmpcat services in `sharedServices`, then loads the
/// built-in catalogues.
bool registerXsmpcatServices(cursiv::SharedCoreServices& sharedServices);

}  // namespace xsmp
