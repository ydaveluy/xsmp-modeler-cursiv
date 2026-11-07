#include "DocumentGenerator.hpp"

#include <algorithm>
#include <cursiv/utils/UriUtils.hpp>
#include <cursiv/workspace/Document.hpp>
#include <cursiv/workspace/Documents.hpp>
#include <set>
#include <xsmp/smp/SmpService.hpp>
#include <xsmp/utils/TreeUtils.hpp>
#include <xsmp/workspace/SourceUri.hpp>

#include "Json.hpp"

namespace xsmp {

namespace {

// The names the project selects as it wrote them, which can be aliases.
std::vector<std::string> projectContributions(const Project& project) {
  std::vector<std::string> ids;
  for (const auto* element : project.elements) {
    if (const auto* profile = cursiv::ast_cast<ProfileReference>(element))
      ids.emplace_back(profile->profile.refText);
    if (const auto* tool = cursiv::ast_cast<ToolReference>(element))
      ids.emplace_back(tool->tool.refText);
  }
  return ids;
}

// What `found` says about the source folders of `project` and of its
// dependencies.
std::vector<std::pair<std::string, std::string>> inProjectSources(
    const XsmpSharedCoreServices& shared, const Project& project,
    const std::vector<std::pair<std::string, std::string>>& found) {
  const auto closure = shared.projectManager->getDependencies(project);
  std::set<std::string> sourceFolders;
  for (const auto* candidate : closure) {
    const auto* document = candidate->getDocument();
    if (!document) continue;
    const auto home = cursiv::file_uri_to_path(document->uri);
    if (!home) continue;
    const auto directory = std::filesystem::path(*home).parent_path();
    const auto projectUri = cursiv::path_to_file_uri(directory.string());
    for (const auto* element : candidate->elements)
      if (const auto* source = cursiv::ast_cast<Source>(element);
          source && !source->path.empty()) {
        const auto folder = resolveSourceUri(projectUri, source->path);
        if (cursiv::contains_uri(projectUri, folder))
          sourceFolders.insert(folder);
      }
  }

  std::vector<std::pair<std::string, std::string>> result;
  for (const auto& item : found) {
    if (!cursiv::is_file_uri(item.first)) continue;
    for (const auto& folder : sourceFolders) {
      if (cursiv::contains_uri(folder, item.first)) {
        result.push_back(item);
        break;
      }
    }
  }
  return result;
}

}  // namespace

std::vector<cursiv::Document*> projectDocuments(XsmpSharedCoreServices& shared,
                                                const Project& project,
                                                bool dependencies) {
  const auto closure = shared.projectManager->getDependencies(project);
  std::vector<cursiv::Document*> documents;
  for (const auto& document : shared.documents->all()) {
    const auto* owner = cursiv::ast_cast<Project>(document->parseResult.value);
    if (!owner) owner = shared.projectManager->getProject(*document);
    if (owner && (dependencies ? closure.contains(owner) : owner == &project))
      documents.push_back(document.get());
  }
  std::ranges::sort(documents, {}, &cursiv::Document::uri);
  return documents;
}

std::vector<std::pair<std::string, std::string>> projectSourceErrors(
    const XsmpSharedCoreServices& shared, const Project& project) {
  return inProjectSources(shared, project, sourceErrors(shared));
}

std::vector<std::pair<std::string, std::string>> projectSourceWarnings(
    const XsmpSharedCoreServices& shared, const Project& project) {
  return inProjectSources(shared, project, sourceWarnings(shared));
}

Json generateProject(XsmpSharedCoreServices& shared, const Project& project,
                     const std::filesystem::path& output,
                     const cursiv::CancellationToken& token) {
  const auto name = std::string(project.name);
  if (!project.getDocument())
    throw std::invalid_argument(
        "Project generation requires an attached document");

  auto directory = output;
  if (directory.empty()) {
    const auto path = cursiv::file_uri_to_path(project.getDocument()->uri);
    if (!path)
      return JsonObject{
          {"generatedProjects", JsonArray{}},
          {"skippedProjects", JsonArray{}},
          {"failures",
           JsonArray{JsonObject{{"projectName", name},
                                {"message",
                                 "Project generation requires a local file "
                                 "URI or an explicit output directory."}}}}};
    directory = std::filesystem::path(*path).parent_path();
  }

  // Validate the whole closure afresh: an error anywhere skips the generation.
  auto visible = projectDocuments(shared, project, true);
  for (auto* document : visible)
    if (document->state >= cursiv::DocumentState::IndexedReferences)
      shared.documentBuilder->resetToState(
          *document, cursiv::DocumentState::IndexedReferences);
  shared.documentBuilder->build(visible, {.validation = true}, token);

  std::size_t errors = 0;
  for (const auto* document : visible)
    for (const auto& diagnostic : document->diagnostics)
      if (diagnostic.severity == cursiv::DiagnosticSeverity::Error) ++errors;
  errors += projectSourceErrors(shared, project).size();
  if (errors)
    return JsonObject{
        {"generatedProjects", JsonArray{}},
        {"skippedProjects",
         JsonArray{JsonObject{{"projectName", name}, {"errorCount", errors}}}},
        {"failures", JsonArray{}}};

  const auto identifiers = projectContributions(project);
  auto& contributions = registry(shared);
  try {
    GenerationGroup group{
        .directory = directory,
        .identifiers = {identifiers.begin(), identifiers.end()}};
    for (const auto* document : projectDocuments(shared, project, false)) {
      cursiv::throw_if_cancelled(token);
      const auto* node = document->parseResult.value;
      if (!node || !cursiv::ast_cast<Document>(node)) continue;
      group.nodes.push_back(node);
    }
    // The last point to stop: past it the generation goes to its end, as generated
    // files left half done would match neither the old model nor the new one. An
    // edit waits for it, since this runs in a write of the workspace lock.
    cursiv::throw_if_cancelled(token);
    contributions.generate(std::span(&group, 1));
  } catch (const cursiv::OperationCancelled&) {
    throw;
  } catch (const std::exception& error) {
    return JsonObject{
        {"generatedProjects", JsonArray{}},
        {"skippedProjects", JsonArray{}},
        {"failures",
         JsonArray{JsonObject{{"projectName", name},
                              {"message", std::string(error.what())}}}}};
  }
  return JsonObject{{"generatedProjects", JsonArray{name}},
                    {"skippedProjects", JsonArray{}},
                    {"failures", JsonArray{}}};
}

}  // namespace xsmp
