#include <cursiv/services/LspServices.hpp>
#include <cursiv/services/ServiceRegistry.hpp>
#include <memory>
#include <utility>
#include <xsmp/DeploymentLspModule.hpp>
#include <xsmp/XsmpCodeActionProvider.hpp>
#include <xsmp/XsmpDefinitionProvider.hpp>
#include <xsmp/XsmpDocumentSymbolProvider.hpp>
#include <xsmp/XsmpRenameProvider.hpp>
#include <xsmp/XsmpasbCompletionProvider.hpp>
#include <xsmp/XsmpasbCoreModule.hpp>
#include <xsmp/XsmpasbFormatter.hpp>
#include <xsmp/XsmpcfgCompletionProvider.hpp>
#include <xsmp/XsmpcfgCoreModule.hpp>
#include <xsmp/XsmpcfgFormatter.hpp>
#include <xsmp/XsmplnkCompletionProvider.hpp>
#include <xsmp/XsmplnkCoreModule.hpp>
#include <xsmp/XsmplnkFormatter.hpp>
#include <xsmp/XsmpsedCompletionProvider.hpp>
#include <xsmp/XsmpsedCoreModule.hpp>
#include <xsmp/XsmpsedFormatter.hpp>

namespace xsmp {

namespace {

// The deployment languages differ in their core services, their formatter and
// their completion.
template <class Formatter, class CompletionProvider, class Services,
          class InstallCore>
std::unique_ptr<Services> createServices(XsmpSharedServices& sharedServices,
                                         std::string languageId,
                                         InstallCore installCore) {
  auto services = std::make_unique<Services>(sharedServices);
  services->languageMetaData.languageId = std::move(languageId);
  cursiv::install_default_lsp_services(*services);
  installCore(*services);
  services->references = std::make_unique<XsmpDeploymentReferences>(*services);
  services->documentSymbolProvider =
      std::make_unique<XsmpDocumentSymbolProvider>(*services);
  services->definitionProvider =
      std::make_unique<XsmpDefinitionProvider>(*services);
  services->codeActionProvider = std::make_unique<XsmpCodeActionProvider>();
  services->renameProvider = std::make_unique<XsmpRenameProvider>(*services);
  services->formatter = std::make_unique<Formatter>(*services);
  services->completionProvider =
      std::make_unique<CompletionProvider>(*services);
  return services;
}

}  // namespace

std::unique_ptr<XsmpasbServices> createXsmpasbServices(
    XsmpSharedServices& sharedServices, std::string languageId) {
  return createServices<XsmpasbFormatter, XsmpasbCompletionProvider,
                        XsmpasbServices>(sharedServices, std::move(languageId),
                                         installXsmpasbCoreModule);
}

std::unique_ptr<XsmpcfgServices> createXsmpcfgServices(
    XsmpSharedServices& sharedServices, std::string languageId) {
  return createServices<XsmpcfgFormatter, XsmpcfgCompletionProvider,
                        XsmpcfgServices>(sharedServices, std::move(languageId),
                                         installXsmpcfgCoreModule);
}

std::unique_ptr<XsmplnkServices> createXsmplnkServices(
    XsmpSharedServices& sharedServices, std::string languageId) {
  return createServices<XsmplnkFormatter, XsmplnkCompletionProvider,
                        XsmplnkServices>(sharedServices, std::move(languageId),
                                         installXsmplnkCoreModule);
}

std::unique_ptr<XsmpsedServices> createXsmpsedServices(
    XsmpSharedServices& sharedServices, std::string languageId) {
  return createServices<XsmpsedFormatter, XsmpsedCompletionProvider,
                        XsmpsedServices>(sharedServices, std::move(languageId),
                                         installXsmpsedCoreModule);
}

bool registerXsmpasbServices(cursiv::SharedLspServices& sharedServices) {
  auto& xsmpShared = dynamic_cast<XsmpSharedServices&>(sharedServices);
  sharedServices.serviceRegistry->registerServices(
      createXsmpasbServices(xsmpShared));
  return true;
}

bool registerXsmpcfgServices(cursiv::SharedLspServices& sharedServices) {
  auto& xsmpShared = dynamic_cast<XsmpSharedServices&>(sharedServices);
  sharedServices.serviceRegistry->registerServices(
      createXsmpcfgServices(xsmpShared));
  return true;
}

bool registerXsmplnkServices(cursiv::SharedLspServices& sharedServices) {
  auto& xsmpShared = dynamic_cast<XsmpSharedServices&>(sharedServices);
  sharedServices.serviceRegistry->registerServices(
      createXsmplnkServices(xsmpShared));
  return true;
}

bool registerXsmpsedServices(cursiv::SharedLspServices& sharedServices) {
  auto& xsmpShared = dynamic_cast<XsmpSharedServices&>(sharedServices);
  sharedServices.serviceRegistry->registerServices(
      createXsmpsedServices(xsmpShared));
  return true;
}

}  // namespace xsmp
