#include <cursiv/services/ServiceRegistry.hpp>
#include <memory>
#include <utility>
#include <xsmp/SharedCoreServices.hpp>
#include <xsmp/XsmpprojectCoreModule.hpp>
#include <xsmp/generated/Xsmpproject/Module.hpp>
#include <xsmp/references/XsmpLinker.hpp>
#include <xsmp/references/XsmpprojectScopeComputation.hpp>
#include <xsmp/validation/XsmpDocumentValidator.hpp>
#include <xsmp/validation/XsmpprojectValidator.hpp>

namespace xsmp {

void installXsmpprojectCoreModule(XsmpprojectCoreServices& services) {
  generated::Xsmpproject::install_generated_module(services);
  services.linker = std::make_unique<XsmpLinker>(services);
  services.languageMetaData.fileNames = {"xsmp.project"};
  services.languageMetaData.fileExtensions = {".xsmpprofile", ".xsmptool"};
  services.scopeComputation =
      std::make_unique<XsmpprojectScopeComputation>(services);
  services.validator = std::make_unique<XsmpprojectValidator>(services);
  services.documentValidator =
      std::make_unique<XsmpDocumentValidator>(services);
  registerXsmpprojectValidationChecks(services);
}

std::unique_ptr<XsmpprojectCoreServices> createXsmpprojectServices(
    XsmpSharedCoreServices& sharedServices, std::string languageId) {
  auto services = cursiv::make_default_core_services<XsmpprojectCoreServices>(
      sharedServices, std::move(languageId));
  installXsmpprojectCoreModule(*services);
  return services;
}

bool registerXsmpprojectServices(cursiv::SharedCoreServices& sharedServices) {
  auto& xsmpShared = dynamic_cast<XsmpSharedCoreServices&>(sharedServices);
  sharedServices.serviceRegistry->registerServices(
      createXsmpprojectServices(xsmpShared));
  return true;
}

}  // namespace xsmp
