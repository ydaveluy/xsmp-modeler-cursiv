#include <algorithm>
#include <cursiv/services/Diagnostic.hpp>
#include <cursiv/services/SharedCoreServices.hpp>
#include <cursiv/syntax-tree/AstNode.hpp>
#include <cursiv/utils/UriUtils.hpp>
#include <cursiv/workspace/Document.hpp>
#include <cursiv/workspace/Documents.hpp>
#include <cursiv/workspace/FileSystemProvider.hpp>
#include <cursiv/workspace/IndexManager.hpp>
#include <span>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>
#include <xsmp/SharedCoreServices.hpp>
#include <xsmp/utils/TreeUtils.hpp>
#include <xsmp/validation/XsmpprojectValidator.hpp>
#include <xsmp/workspace/ProjectManager.hpp>
#include <xsmp/workspace/SourceUri.hpp>

namespace xsmp {

using Acceptor = cursiv::ValidationAcceptor;

namespace {

bool isKnownStandard(std::string_view standard) {
  return std::ranges::find(kSmpStandards, standard) != kSmpStandards.end();
}

// True unless `relPath` escapes the project directory. `.` and empty segments
// are ignored; ordinary segments descend.
bool isContainedRelPath(std::string_view relPath) {
  auto depth = 0;
  std::size_t i = 0;
  while (i <= relPath.size()) {
    auto j = i;
    while (j < relPath.size() && relPath[j] != '/' && relPath[j] != '\\') {
      ++j;
    }
    const auto seg = relPath.substr(i, j - i);
    if (seg == "..") {
      if (--depth < 0) {
        return false;
      }
    } else if (!seg.empty() && seg != ".") {
      ++depth;
    }
    if (j >= relPath.size()) {
      break;
    }
    i = j + 1;
  }
  return true;
}

}  // namespace

XsmpprojectValidator::XsmpprojectValidator(const cursiv::CoreServices& services)
    : _services(services) {}

void XsmpprojectValidator::checkProfile(const Profile& profile,
                                        Acceptor& accept) const {
  if (!documentOf(profile).uri.ends_with(".xsmpprofile")) {
    accept
        .error(profile,
               "A profile file shall have '.xsmpprofile' file extension.")
        .keyword("profile");
  }
}

void XsmpprojectValidator::checkTool(const Tool& tool, Acceptor& accept) const {
  if (!documentOf(tool).uri.ends_with(".xsmptool")) {
    accept.error(tool, "A tool file shall have '.xsmptool' file extension.")
        .keyword("tool");
  }
}

void XsmpprojectValidator::checkProject(const Project& project,
                                        Acceptor& accept) const {
  if (cursiv::uri_file_name(documentOf(project).uri) != "xsmp.project") {
    accept.error(project, "A project file name shall be 'xsmp.project'.")
        .keyword("project");
  }

  if (!project.standard.empty() && !isKnownStandard(project.standard)) {
    accept
        .error(project,
               "Unknown version. Only the following versions are "
               "supported: ECSS_SMP_2020, ECSS_SMP_2025.")
        .property<&Project::standard>();
  }

  if (!project.name.empty()) {
    const auto& documents = *_services.shared().documents;
    // Point the diagnostic at every other project sharing this name (their name
    // segment and document URI).
    std::vector<cursiv::DiagnosticRelatedInformation> related;
    (void)_services.shared().indexManager->visitByName(
        project.name, true, {}, *_services.astReflection,
        [&](const cursiv::AstNodeDescription& description) {
          if (!description.type
              || !description.type->derivesFrom(&cursiv::ast_type_tag<Project>))
            return true;
          const auto& node = resolveNode(documents, description);
          // The project being validated is itself indexed.
          if (&node == &project) return true;
          const auto [begin, end] = rangeForFeature(node, "name", 0);
          related.push_back({.uri = std::string(documentOf(node).uri),
                             .message = std::string(project.name),
                             .range = {.begin = begin, .end = end}});
          return true;
        });
    if (!related.empty()) {
      accept.error(project, "Duplicated project name")
          .property<&Project::name>()
          .relatedInformation(
              std::span<const cursiv::DiagnosticRelatedInformation>(related));
    }
  }

  const std::string_view projectUri = documentOf(project).uri;
  const auto* shared =
      dynamic_cast<const XsmpSharedCoreServices*>(&_services.shared());
  auto hasProfile = false;
  std::unordered_set<std::string> tools;
  std::unordered_set<const Project*> dependencies;

  for (const auto& element : project.elements) {
    const auto* node = element;
    if (const auto* profileRef =
            cursiv::ast_cast<const ProfileReference>(node)) {
      const auto* target = profileRef->profile.get();
      if (!target) {
        continue;
      }

      if (shared
          && shared->contributionDeprecatedAliases.contains(
              std::string(profileRef->profile.refText)))
        accept
            .warning(*profileRef, "Deprecated: Use the \""
                                      + std::string(target->name)
                                      + "\" profile instead.")
            .property<&ProfileReference::profile>()
            .tags({cursiv::DiagnosticTag::Deprecated});

      if (hasProfile) {
        accept.error(*profileRef, "A profile is already defined.")
            .property<&ProfileReference::profile>();
      } else {
        hasProfile = true;
      }
    } else if (const auto* toolRef =
                   cursiv::ast_cast<const ToolReference>(node)) {
      const auto* target = toolRef->tool.get();
      if (!target) {
        continue;
      }

      if (shared
          && shared->contributionDeprecatedAliases.contains(
              std::string(toolRef->tool.refText)))
        accept
            .warning(*toolRef, "Deprecated: Use the \""
                                   + std::string(target->name)
                                   + "\" tool instead.")
            .property<&ToolReference::tool>()
            .tags({cursiv::DiagnosticTag::Deprecated});

      if (!tools.insert(std::string(target->name)).second) {
        accept
            .error(*toolRef,
                   "Duplicated tool '" + std::string(target->name) + "'.")
            .property<&ToolReference::tool>();
      }
    } else if (const auto* dependency =
                   cursiv::ast_cast<const Dependency>(node)) {
      const auto* target = dependency->project.get();
      if (!target) {
        continue;
      }

      // `getDependencies` is transitive and holds `target` itself.
      if (shared && shared->projectManager
          && shared->projectManager->getDependencies(*target).contains(
              &project)) {
        accept
            .error(*dependency, "Cyclic dependency detected '"
                                    + std::string(target->name) + "'.")
            .property<&Dependency::project>();
      }
      if (!dependencies.insert(target).second) {
        accept
            .error(*dependency,
                   "Duplicated dependency '" + std::string(target->name) + "'.")
            .property<&Dependency::project>();
      }
      if (project.standard != target->standard) {
        accept
            .error(*dependency, "SMP version does not match '"
                                    + std::string(project.standard) + "'.")
            .property<&Dependency::project>();
      }
    } else if (const auto* source = cursiv::ast_cast<const Source>(node)) {
      if (source->path.empty()) {
        continue;
      }
      if (!isContainedRelPath(source->path)) {
        accept
            .error(*source, "Source path '" + std::string(source->path) +
                                "' is not contained within the project "
                                "directory.")
            .property<&Source::path>();
      } else {
        // Through `FileSystemProvider`, the abstraction the CLI and the LSP use
        // to load files, so this works whether the workspace is on disk or in
        // memory (an `EmptyFileSystemProvider` reports every path as absent).
        const auto sourceUri =
            resolveSourceUri(dirnameOfUri(projectUri), source->path);
        if (!_services.shared().fileSystemProvider->exists(sourceUri)) {
          accept
              .error(*source, "Source path '" + std::string(source->path)
                                  + "' does not exist.")
              .property<&Source::path>();
        }
      }
    }
  }
}

}  // namespace xsmp
