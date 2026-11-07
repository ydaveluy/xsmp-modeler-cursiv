#pragma once

#include <cursiv/services/SharedCoreServices.hpp>
#include <memory>
#include <string>
#include <xsmp/XsmpprojectCoreServices.hpp>

namespace xsmp {

/// Applies the `xsmp.project` core overrides (parser, file matching) on top of
/// cursiv's default core services.
void installXsmpprojectCoreModule(XsmpprojectCoreServices& services);

/// Builds the core-only `xsmp.project` language services.
std::unique_ptr<XsmpprojectCoreServices> createXsmpprojectServices(
    XsmpSharedCoreServices& sharedServices,
    std::string languageId = "xsmpproject");

/// Registers the core-only `xsmp.project` services in `sharedServices`.
bool registerXsmpprojectServices(cursiv::SharedCoreServices& sharedServices);

}  // namespace xsmp
