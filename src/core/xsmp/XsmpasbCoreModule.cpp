#include <cursiv/services/ServiceRegistry.hpp>
#include <memory>
#include <utility>
#include <xsmp/XsmpasbCoreModule.hpp>
#include <xsmp/generated/Xsmpasb/Module.hpp>
#include <xsmp/references/XsmpLinker.hpp>
#include <xsmp/references/XsmpasbScopeComputation.hpp>
#include <xsmp/references/XsmpasbScopeProvider.hpp>
#include <xsmp/validation/DeploymentDocumentValidator.hpp>

namespace xsmp {

void installXsmpasbCoreModule(XsmpasbCoreServices& services) {
  generated::Xsmpasb::install_generated_module(services);
  services.linker = std::make_unique<XsmpLinker>(services);
  services.scopeComputation =
      std::make_unique<XsmpasbScopeComputation>(services);
  services.scopeProvider = std::make_unique<XsmpasbScopeProvider>(services);
  services.validator = std::make_unique<XsmpasbValidator>();
  services.documentValidator =
      std::make_unique<DeploymentDocumentValidator>(services);
  registerXsmpasbValidationChecks(services);
}

std::unique_ptr<XsmpasbCoreServices> createXsmpasbServices(
    XsmpSharedCoreServices& sharedServices, std::string languageId) {
  auto services = cursiv::make_default_core_services<XsmpasbCoreServices>(
      sharedServices, std::move(languageId));
  installXsmpasbCoreModule(*services);
  return services;
}

bool registerXsmpasbServices(cursiv::SharedCoreServices& sharedServices) {
  auto& xsmpShared = dynamic_cast<XsmpSharedCoreServices&>(sharedServices);
  sharedServices.serviceRegistry->registerServices(
      createXsmpasbServices(xsmpShared));
  return true;
}

}  // namespace xsmp
