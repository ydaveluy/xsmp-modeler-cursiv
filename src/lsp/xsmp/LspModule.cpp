#include <memory>
#include <xsmp/Contributions.hpp>
#include <xsmp/LspModule.hpp>
#include <xsmp/SharedLspServices.hpp>
#include <xsmp/XsmpLanguageServer.hpp>
#include <xsmp/XsmpNodeKindProvider.hpp>
#include <xsmp/XsmpTextDocumentContentProvider.hpp>
#include <xsmp/smp/SmpService.hpp>
#include <xsmp/workspace/ProjectManager.hpp>

namespace xsmp {

bool registerXsmpServices(cursiv::SharedLspServices& sharedServices) {
  sharedServices.nodeKindProvider = std::make_unique<XsmpNodeKindProvider>();
  return registerXsmpcatServices(sharedServices)
         && registerXsmpprojectServices(sharedServices)
         && registerXsmpasbServices(sharedServices)
         && registerXsmpcfgServices(sharedServices)
         && registerXsmplnkServices(sharedServices)
         && registerXsmpsedServices(sharedServices);
}

bool registerLanguageServer(cursiv::SharedCoreServices& shared) {
  auto& xsmpShared = dynamic_cast<XsmpSharedServices&>(shared);
  xsmpShared.projectManager = std::make_unique<ProjectManager>(xsmpShared);
  xsmpShared.textDocumentContentProvider =
      std::make_unique<XsmpTextDocumentContentProvider>(xsmpShared);
  // Before the languages: a completion provider, once created, subscribes to
  // the handshake of this server.
  xsmpShared.languageServer = std::make_unique<XsmpLanguageServer>(xsmpShared);
  if (!registerXsmpServices(
          static_cast<cursiv::SharedLspServices&>(xsmpShared)))
    return false;
  // The builtin descriptors are documents of these languages.
  loadBuiltinDescriptors(xsmpShared);
  installSmpWorkspaceManager(xsmpShared);
  return true;
}

}  // namespace xsmp
