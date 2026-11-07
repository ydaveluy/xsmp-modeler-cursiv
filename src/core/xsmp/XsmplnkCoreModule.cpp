#include <cursiv/services/ServiceRegistry.hpp>
#include <memory>
#include <utility>
#include <xsmp/XsmplnkCoreModule.hpp>
#include <xsmp/generated/Xsmplnk/Module.hpp>
#include <xsmp/references/XsmpLinker.hpp>
#include <xsmp/references/XsmplnkScopeComputation.hpp>
#include <xsmp/references/XsmplnkScopeProvider.hpp>
#include <xsmp/validation/DeploymentDocumentValidator.hpp>

namespace xsmp {

void installXsmplnkCoreModule(XsmplnkCoreServices& services) {
  generated::Xsmplnk::install_generated_module(services);
  services.linker = std::make_unique<XsmpLinker>(services);
  services.scopeComputation =
      std::make_unique<XsmplnkScopeComputation>(services);
  services.scopeProvider = std::make_unique<XsmplnkScopeProvider>(services);
  services.validator = std::make_unique<XsmplnkValidator>();
  services.documentValidator =
      std::make_unique<DeploymentDocumentValidator>(services);
  registerXsmplnkValidationChecks(services);
}

std::unique_ptr<XsmplnkCoreServices> createXsmplnkServices(
    XsmpSharedCoreServices& sharedServices, std::string languageId) {
  auto services = cursiv::make_default_core_services<XsmplnkCoreServices>(
      sharedServices, std::move(languageId));
  installXsmplnkCoreModule(*services);
  return services;
}

bool registerXsmplnkServices(cursiv::SharedCoreServices& sharedServices) {
  auto& xsmpShared = dynamic_cast<XsmpSharedCoreServices&>(sharedServices);
  sharedServices.serviceRegistry->registerServices(
      createXsmplnkServices(xsmpShared));
  return true;
}

}  // namespace xsmp
