#include <cursiv/services/ServiceRegistry.hpp>
#include <memory>
#include <utility>
#include <xsmp/XsmpsedCoreModule.hpp>
#include <xsmp/generated/Xsmpsed/Module.hpp>
#include <xsmp/references/XsmpLinker.hpp>
#include <xsmp/references/XsmpsedScopeComputation.hpp>
#include <xsmp/references/XsmpsedScopeProvider.hpp>
#include <xsmp/validation/DeploymentDocumentValidator.hpp>

namespace xsmp {

void installXsmpsedCoreModule(XsmpsedCoreServices& services) {
  generated::Xsmpsed::install_generated_module(services);
  services.linker = std::make_unique<XsmpLinker>(services);
  services.scopeComputation =
      std::make_unique<XsmpsedScopeComputation>(services);
  services.scopeProvider = std::make_unique<XsmpsedScopeProvider>(services);
  services.validator = std::make_unique<XsmpsedValidator>();
  services.documentValidator =
      std::make_unique<DeploymentDocumentValidator>(services);
  registerXsmpsedValidationChecks(services);
}

std::unique_ptr<XsmpsedCoreServices> createXsmpsedServices(
    XsmpSharedCoreServices& sharedServices, std::string languageId) {
  auto services = cursiv::make_default_core_services<XsmpsedCoreServices>(
      sharedServices, std::move(languageId));
  installXsmpsedCoreModule(*services);
  return services;
}

bool registerXsmpsedServices(cursiv::SharedCoreServices& sharedServices) {
  auto& xsmpShared = dynamic_cast<XsmpSharedCoreServices&>(sharedServices);
  sharedServices.serviceRegistry->registerServices(
      createXsmpsedServices(xsmpShared));
  return true;
}

}  // namespace xsmp
