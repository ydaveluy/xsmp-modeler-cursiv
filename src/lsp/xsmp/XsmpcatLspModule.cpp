#include <cursiv/services/LspServices.hpp>
#include <cursiv/services/ServiceRegistry.hpp>
#include <memory>
#include <utility>
#include <xsmp/Builtins.hpp>
#include <xsmp/XsmpCodeActionProvider.hpp>
#include <xsmp/XsmpDefinitionProvider.hpp>
#include <xsmp/XsmpDocumentSymbolProvider.hpp>
#include <xsmp/XsmpRenameProvider.hpp>
#include <xsmp/XsmpcatCompletionProvider.hpp>
#include <xsmp/XsmpcatCoreModule.hpp>
#include <xsmp/XsmpcatFoldingRangeProvider.hpp>
#include <xsmp/XsmpcatFormatter.hpp>
#include <xsmp/XsmpcatLspModule.hpp>
#include <xsmp/XsmpcatTypeHierarchyProvider.hpp>

namespace xsmp {

// Installs the xsmpcat providers over the defaults that
// `install_default_lsp_services` wires in, and fills the slots those leave
// empty, such as the type-hierarchy provider.
void installXsmpcatLspModule(XsmpcatServices& services) {
  services.completionProvider =
      std::make_unique<XsmpcatCompletionProvider>(services, services);
  services.codeActionProvider = std::make_unique<XsmpCodeActionProvider>();
  services.formatter = std::make_unique<XsmpcatFormatter>(services);
  services.renameProvider = std::make_unique<XsmpRenameProvider>(services);
  services.documentSymbolProvider =
      std::make_unique<XsmpDocumentSymbolProvider>(services);
  services.typeDefinitionProvider =
      std::make_unique<XsmpcatTypeDefinitionProvider>(services);
  services.foldingRangeProvider =
      std::make_unique<XsmpcatFoldingRangeProvider>(services);
  services.typeHierarchyProvider =
      std::make_unique<XsmpcatTypeHierarchyProvider>(services);
}

std::unique_ptr<XsmpcatServices> createXsmpcatServices(
    XsmpSharedServices& sharedServices, std::string languageId) {
  auto services = std::make_unique<XsmpcatServices>(sharedServices);
  services->languageMetaData.languageId = std::move(languageId);
  cursiv::install_default_lsp_services(*services);
  installXsmpcatCoreModule(*services);
  installXsmpcatLspModule(*services);
  return services;
}

bool registerXsmpcatServices(cursiv::SharedLspServices& sharedServices) {
  auto& xsmpShared = dynamic_cast<XsmpSharedServices&>(sharedServices);
  sharedServices.serviceRegistry->registerServices(
      createXsmpcatServices(xsmpShared));
  loadXsmpcatBuiltins(sharedServices);
  return true;
}

}  // namespace xsmp
