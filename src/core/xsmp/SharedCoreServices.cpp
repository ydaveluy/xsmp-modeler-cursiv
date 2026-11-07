#include <cursiv/workspace/LocalFileSystemProvider.hpp>
#include <xsmp/SharedCoreServices.hpp>
#include <xsmp/workspace/XsmpDocumentBuilder.hpp>
#include <xsmp/workspace/XsmpServiceRegistry.hpp>

namespace xsmp {

XsmpSharedCoreServices::XsmpSharedCoreServices() {
  documentBuilder = std::make_unique<XsmpDocumentBuilder>(*this);
  serviceRegistry = std::make_unique<XsmpServiceRegistry>(*this);
}

std::unique_ptr<XsmpSharedCoreServices> createXsmpSharedServices() {
  auto shared = std::make_unique<XsmpSharedCoreServices>();
  shared->fileSystemProvider =
      std::make_unique<cursiv::LocalFileSystemProvider>();
  cursiv::install_default_shared_core_services(*shared);
  // After the defaults: it replaces their index manager and listens to their
  // document builder.
  shared->projectManager = std::make_unique<ProjectManager>(*shared);
  return shared;
}

const cursiv::ValidationCategory& contributionCategory(
    XsmpSharedCoreServices& shared, const std::string& id) {
  auto& [name, category] = *shared.contributionCategories.try_emplace(id).first;
  category.description = name.c_str();
  return category;
}

}  // namespace xsmp
