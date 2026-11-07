#include <cursiv/services/ServiceRegistry.hpp>
#include <memory>
#include <utility>
#include <xsmp/Builtins.hpp>
#include <xsmp/XsmpcatCoreModule.hpp>
#include <xsmp/generated/Xsmpcat/Module.hpp>
#include <xsmp/references/QualifiedName.hpp>
#include <xsmp/references/XsmpLinker.hpp>
#include <xsmp/references/XsmpTypeProvider.hpp>
#include <xsmp/references/XsmpcatScopeComputation.hpp>
#include <xsmp/references/XsmpcatScopeProvider.hpp>
#include <xsmp/validation/XsmpDocumentValidator.hpp>
#include <xsmp/validation/XsmpcatValidator.hpp>

namespace xsmp {

void installXsmpcatCoreModule(XsmpcatCoreServices& services) {
  generated::Xsmpcat::install_generated_module(services);
  services.linker = std::make_unique<XsmpLinker>(services);
  services.typeProvider = std::make_unique<const XsmpTypeProvider>(services);
  services.scopeComputation =
      std::make_unique<XsmpcatScopeComputation>(services);
  services.scopeProvider = std::make_unique<XsmpcatScopeProvider>(services);
  services.validator = std::make_unique<XsmpcatValidator>(services);
  services.documentValidator =
      std::make_unique<XsmpDocumentValidator>(services);
  registerXsmpcatValidationChecks(services);
}

std::unique_ptr<XsmpcatCoreServices> createXsmpcatServices(
    XsmpSharedCoreServices& sharedServices, std::string languageId) {
  auto services = cursiv::make_default_core_services<XsmpcatCoreServices>(
      sharedServices, std::move(languageId));
  installXsmpcatCoreModule(*services);
  return services;
}

bool registerXsmpcatServices(cursiv::SharedCoreServices& sharedServices) {
  auto& xsmpShared = dynamic_cast<XsmpSharedCoreServices&>(sharedServices);
  sharedServices.serviceRegistry->registerServices(
      createXsmpcatServices(xsmpShared));
  loadXsmpcatBuiltins(sharedServices);
  return true;
}

}  // namespace xsmp
