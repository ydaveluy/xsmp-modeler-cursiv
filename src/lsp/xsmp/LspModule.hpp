#pragma once

#include <cursiv/services/SharedLspServices.hpp>
#include <xsmp/DeploymentLspModule.hpp>
#include <xsmp/XsmpcatLspModule.hpp>
#include <xsmp/XsmpprojectLspModule.hpp>

namespace xsmp {

/// Registers the six XSMP editor languages in their shared workspace.
bool registerXsmpServices(cursiv::SharedLspServices& sharedServices);

/// Installs what a language server needs on top of those languages.
bool registerLanguageServer(cursiv::SharedCoreServices& shared);

}  // namespace xsmp
