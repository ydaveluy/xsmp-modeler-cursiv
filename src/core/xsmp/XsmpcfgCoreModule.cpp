#include <cursiv/services/ServiceRegistry.hpp>
#include <memory>
#include <utility>
#include <xsmp/XsmpcfgCoreModule.hpp>
#include <xsmp/generated/Xsmpcfg/Module.hpp>
#include <xsmp/references/XsmpLinker.hpp>
#include <xsmp/references/XsmpcfgScopeComputation.hpp>
#include <xsmp/references/XsmpcfgScopeProvider.hpp>
#include <xsmp/validation/DeploymentDocumentValidator.hpp>

namespace xsmp {

void installXsmpcfgCoreModule(XsmpcfgCoreServices& services) {
  generated::Xsmpcfg::install_generated_module(services);
  services.linker = std::make_unique<XsmpLinker>(services);
  services.scopeComputation =
      std::make_unique<XsmpcfgScopeComputation>(services);
  services.scopeProvider = std::make_unique<XsmpcfgScopeProvider>(services);
  services.validator = std::make_unique<XsmpcfgValidator>();
  services.documentValidator =
      std::make_unique<DeploymentDocumentValidator>(services);
  registerXsmpcfgValidationChecks(services);
}

std::unique_ptr<XsmpcfgCoreServices> createXsmpcfgServices(
    XsmpSharedCoreServices& sharedServices, std::string languageId) {
  auto services = cursiv::make_default_core_services<XsmpcfgCoreServices>(
      sharedServices, std::move(languageId));
  installXsmpcfgCoreModule(*services);
  return services;
}

bool registerXsmpcfgServices(cursiv::SharedCoreServices& sharedServices) {
  auto& xsmpShared = dynamic_cast<XsmpSharedCoreServices&>(sharedServices);
  sharedServices.serviceRegistry->registerServices(
      createXsmpcfgServices(xsmpShared));
  return true;
}

}  // namespace xsmp
