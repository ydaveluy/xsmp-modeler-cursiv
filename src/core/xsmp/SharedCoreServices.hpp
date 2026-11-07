#pragma once

#include <cursiv/validation/ValidationCategory.hpp>
#include <functional>
#include <map>
#include <cursiv/services/SharedCoreServices.hpp>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <xsmp/workspace/ProjectManager.hpp>

namespace xsmp {

class Registry;

/// Workspace-wide shared services reused by all six XSMP languages:
/// everything from `cursiv::SharedCoreServices` plus the project-visibility
/// `ProjectManager` and the profiles and tools that extensions contribute.
struct XsmpSharedCoreServices : virtual cursiv::SharedCoreServices {
  // Installs the document builder, before anything can listen to another one,
  // and the service registry, which knows the languages of the SMP mirrors.
  XsmpSharedCoreServices();

  std::unique_ptr<ProjectManager> projectManager;
  std::shared_ptr<Registry> contributionRegistry;

  // The profile or tool that provides each payload document, by document URI.
  std::unordered_map<std::string, std::string> contributionPayloadOwners;

  // Every profile or tool that provides a payload, when several declare it.
  std::unordered_map<std::string, std::unordered_set<std::string>>
      contributionPayloadContributors;

  // The contribution each alias names, and the aliases that are deprecated.
  std::unordered_map<std::string, std::string> contributionAliases;
  std::unordered_set<std::string> contributionDeprecatedAliases;

  // The validation category of each profile or tool that has checks, by id. A
  // document runs those of the contributions its project selects.
  std::map<std::string, cursiv::ValidationCategory, std::less<>>
      contributionCategories;
};

/// The validation category of the contribution `id`, which its checks register
/// under. Its address is its identity.
const cursiv::ValidationCategory& contributionCategory(
    XsmpSharedCoreServices& shared, const std::string& id);

/// Builds and fully initializes the core-only xsmp shared services (installs
/// the cursiv defaults, then constructs `projectManager`). The LSP server and
/// tests build the LSP-enabled `xsmp::XsmpSharedServices` the same way. A
/// container built any other way leaves `projectManager` null.
[[nodiscard]] std::unique_ptr<XsmpSharedCoreServices>
createXsmpSharedServices();

}  // namespace xsmp
