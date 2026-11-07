#include <cursiv/services/LspServices.hpp>
#include <cursiv/services/ServiceRegistry.hpp>
#include <memory>
#include <utility>
#include <xsmp/XsmpDocumentSymbolProvider.hpp>
#include <xsmp/XsmpprojectCompletionProvider.hpp>
#include <xsmp/XsmpprojectCoreModule.hpp>
#include <xsmp/XsmpprojectFormatter.hpp>
#include <xsmp/XsmpprojectLspModule.hpp>
#include <xsmp/XsmpprojectRenameProvider.hpp>

namespace xsmp {

void installXsmpprojectLspModule(XsmpprojectServices& services) {
  services.formatter = std::make_unique<XsmpprojectFormatter>(services);
  services.renameProvider =
      std::make_unique<XsmpprojectRenameProvider>(services);
  services.completionProvider =
      std::make_unique<XsmpprojectCompletionProvider>(services);
  services.documentSymbolProvider =
      std::make_unique<XsmpDocumentSymbolProvider>(services);
}

std::unique_ptr<XsmpprojectServices> createXsmpprojectServices(
    XsmpSharedServices& sharedServices, std::string languageId) {
  auto services = std::make_unique<XsmpprojectServices>(sharedServices);
  services->languageMetaData.languageId = std::move(languageId);
  cursiv::install_default_lsp_services(*services);
  installXsmpprojectCoreModule(*services);
  installXsmpprojectLspModule(*services);
  return services;
}

bool registerXsmpprojectServices(cursiv::SharedLspServices& sharedServices) {
  auto& xsmpShared = dynamic_cast<XsmpSharedServices&>(sharedServices);
  sharedServices.serviceRegistry->registerServices(
      createXsmpprojectServices(xsmpShared));
  return true;
}

}  // namespace xsmp
