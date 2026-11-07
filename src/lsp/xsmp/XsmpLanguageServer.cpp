#include <algorithm>
#include <cctype>
#include <chrono>
#include <cursiv/services/ServiceRegistry.hpp>
#include <cursiv/utils/UriUtils.hpp>
#include <cursiv/workspace/Document.hpp>
#include <cursiv/workspace/Documents.hpp>
#include <cursiv/workspace/WorkspaceLock.hpp>
#include <cursiv/workspace/WorkspaceManager.hpp>
#include <filesystem>
#include <stdexcept>
#include <xsmp/Contributions.hpp>
#include <xsmp/DocumentGenerator.hpp>
#include <xsmp/Json.hpp>
#include <xsmp/ProjectCreation.hpp>
#include <xsmp/XsmpLanguageServer.hpp>
#include <xsmp/XsmpTextDocumentContentProvider.hpp>
#include <xsmp/ast.hpp>
#include <xsmp/smp/SmpService.hpp>
#include <xsmp/workspace/SourceUri.hpp>

namespace xsmp {

namespace {

namespace fs = std::filesystem;

// Waits for the lock in short steps, so that a cancelled request stops waiting.
// Throws when cancelled, also once the lock is held.
auto generationLock(std::timed_mutex& mutex,
                    const cursiv::CancellationToken& token) {
  std::unique_lock lock(mutex, std::defer_lock);
  while (!lock.try_lock_for(std::chrono::milliseconds(10)))
    cursiv::throw_if_cancelled(token);
  cursiv::throw_if_cancelled(token);
  return lock;
}

// A local path from a `file:` URI or a plain path. Throws on a URI of another
// scheme; a single letter before the colon is a Windows drive, not a scheme.
fs::path pathOf(std::string_view uri) {
  if (auto path = cursiv::file_uri_to_path(uri)) return *path;
  const auto colon = uri.find(':');
  const auto schemeCharacter = [](unsigned char c) {
    return std::isalnum(c) || c == '+' || c == '-' || c == '.';
  };
  if (colon != std::string_view::npos && colon > 1
      && std::isalpha(static_cast<unsigned char>(uri.front()))
      && std::all_of(uri.begin() + 1, uri.begin() + colon, schemeCharacter))
    throw std::invalid_argument("Expected a local file URI");
  return fs::path(uri);
}

bool isSmpFile(std::string_view uri) {
  const auto extension = cursiv::uri_extension(uri);
  return extension == ".smpcat" || extension == ".smpcfg"
         || extension == ".smpasb" || extension == ".smplnk"
         || extension == ".smpsed";
}

// The project a document declares, else the one it belongs to. Null if none.
const Project* projectOf(const XsmpSharedServices& shared,
                         const cursiv::Document& document) {
  if (const auto* project =
          cursiv::ast_cast<Project>(document.parseResult.value))
    return project;
  return shared.projectManager->getProject(document);
}

// Generates the project of the document at `uri`, or every project under `all`,
// and merges the reports: `generatedProjects`, `skippedProjects`, `failures`. A
// document in no project is no failure: the report is then empty.
Json generateProjects(XsmpSharedServices& shared, std::string_view uri,
                      bool all, const cursiv::CancellationToken& token) {
  JsonArray generated, skipped, failures;
  // URIs and names only: the writes below can replace the documents.
  std::vector<std::pair<std::string, std::string>> targets;
  (void)shared.workspaceLock->read(
      [&] {
        std::vector<const Project*> projects;
        if (all) {
          projects = shared.projectManager->getProjects();
          std::ranges::sort(projects, {}, &Project::name);
        } else if (const auto selected =
                       uri.empty() ? nullptr
                                   : shared.documents->getDocument(uri)) {
          if (const auto* project = projectOf(shared, *selected))
            projects.push_back(project);
        }
        for (const auto* project : projects)
          targets.emplace_back(project->getDocument()->uri,
                               std::string(project->name));
      },
      token);

  cursiv::throw_if_cancelled(token);
  for (const auto& [projectUri, projectName] : targets) {
    // A build settles its write. A separate durable action per project keeps
    // later resets on the mutation side of that boundary, even for Generate
    // All.
    shared.workspaceLock->write(
        [&](const cursiv::CancellationToken&) {
          cursiv::throw_if_cancelled(token);
          const auto current = shared.documents->getDocument(projectUri);
          const auto* project =
              current ? cursiv::ast_cast<Project>(current->parseResult.value)
                      : nullptr;
          if (!project) {
            skipped.emplace_back(
                JsonObject{{"projectName", projectName}, {"errorCount", 1}});
            return;
          }
          const auto report = generateProject(shared, *project, {}, token);
          const auto append = [&](std::string_view key, JsonArray& target) {
            for (const auto& value : report.object().get(key).array())
              target.push_back(value);
          };
          append("generatedProjects", generated);
          append("skippedProjects", skipped);
          append("failures", failures);
        },
        cursiv::WorkspaceLock::WriteMode::Durable);
  }

  return JsonObject{{"generatedProjects", std::move(generated)},
                    {"skippedProjects", std::move(skipped)},
                    {"failures", std::move(failures)}};
}

// Writes the XSMP form of an SMP file; answers with path, kind and warnings.
Json importSmp(const XsmpSharedServices& shared, const Json& param) {
  if (!param.isObject())
    throw std::invalid_argument(
        "xsmp/importSmpFile expects {uri, outputUri?, force?}");
  const auto& request = param.object();
  const auto input = stringAt(request, "uri");
  const auto output = stringAt(request, "outputUri");
  if (input.empty())
    throw std::invalid_argument("An SMP source URI is required");

  ImportOptions options;
  (void)shared.workspaceLock->read([&] {
    for (const auto& folder : shared.workspaceManager->workspaceFolders())
      if (const auto path = cursiv::file_uri_to_path(folder.uri))
        options.searchRoots.emplace_back(*path);
  });

  const auto imported =
      importFile(pathOf(input), output.empty() ? fs::path{} : pathOf(output),
                 boolAt(request, "force"), options);

  JsonArray warnings;
  for (const auto& warning : imported.warnings) warnings.emplace_back(warning);
  JsonObject result;
  result.insert("outputPath", Json(imported.outputPath.string()));
  result.insert("kind", Json(imported.kind));
  result.insert("warnings", Json(std::move(warnings)));
  return result;
}

void createProjectFrom(const XsmpSharedServices& shared, const Json& param) {
  if (!param.isObject())
    throw std::invalid_argument("A project request is required");
  const auto& request = param.object();
  ScaffoldContext context;
  context.projectName = stringAt(request, "projectName");
  context.projectDir = pathOf(stringAt(request, "projectDir"));
  context.selectedProfileId = stringAt(request, "selectedProfileId");
  context.selectedToolIds = stringsAt(request, "selectedToolIds");
  if (const auto* values = request.find("promptValues"))
    context.promptValues = *values;
  (void)shared.workspaceLock->read(
      [&] { createProject(registry(shared), std::move(context)); });
}

}  // namespace

XsmpLanguageServer::XsmpLanguageServer(XsmpSharedServices& shared)
    : cursiv::DefaultLanguageServer(shared),
      _shared(shared),
      _diagnostics(shared) {
  registerRequests();
}

void XsmpLanguageServer::registerRequests() {
  const auto accept = [this](std::string method, auto operation) {
    onRequest(
        std::move(method),
        [operation = std::move(operation)](
            const Json& param, const cursiv::CancellationToken& token) -> Json {
          cursiv::throw_if_cancelled(token);
          auto result = operation(param, token);
          cursiv::throw_if_cancelled(token);
          return result;
        });
  };

  accept("xsmp/generateProject",
         [this](const Json& param, const cursiv::CancellationToken& token) {
           const auto serial = generationLock(_generationMutex, token);
           return generateProjects(
               _shared, param.isString() ? param.string() : std::string{},
               false, token);
         });

  accept("xsmp/generateAllProjects",
         [this](const Json&, const cursiv::CancellationToken& token) {
           const auto serial = generationLock(_generationMutex, token);
           return generateProjects(_shared, {}, true, token);
         });

  accept("xsmp/importSmpFile",
         [this](const Json& param, const cursiv::CancellationToken&) {
           return importSmp(_shared, param);
         });

  accept("xsmp/createProject",
         [this](const Json& param, const cursiv::CancellationToken&) -> Json {
           createProjectFrom(_shared, param);
           return nullptr;
         });

  accept("xsmp/getContributionSummaries",
         [this](const Json& param, const cursiv::CancellationToken&) {
           Json result;
           const auto kind = param.isString() ? param.string() : std::string{};
           (void)_shared.workspaceLock->read(
               [&] { result = registry(_shared).summaries(kind); });
           return result;
         });

  accept(
      "xsmp/getContributionWizardPrompts",
      [this](const Json& param, const cursiv::CancellationToken&) {
        if (!param.isObject())
          throw std::invalid_argument("A wizard prompts request is required");
        const auto& request = param.object();
        const auto tools = stringsAt(request, "selectedToolIds");
        Json result;
        (void)_shared.workspaceLock->read([&] {
          result = registry(_shared).wizardPrompts(
              stringAt(request, "selectedProfileId"), tools);
        });
        return result;
      });

  accept("xsmp/registerContributions",
         [this](const Json& param, const cursiv::CancellationToken&) {
           // One entry alone reaches here without its array.
           const auto entries =
               param.isArray() ? param : Json(JsonArray{param});
           Json report;
           // A build settles a write, and a registration resets every document
           // after: the placeholders go in a write of their own.
           std::vector<std::string> placeholders;
           (void)_shared.workspaceLock->read([&] {
             placeholders = registry(_shared).placeholders(_shared, entries);
           });
           if (!placeholders.empty())
             _shared.workspaceLock->write(
                 [&](const cursiv::CancellationToken&) {
                   const std::vector<std::string_view> deleted(
                       placeholders.begin(), placeholders.end());
                   _shared.documentBuilder->update({}, deleted);
                 },
                 cursiv::WorkspaceLock::WriteMode::Durable);
           _shared.workspaceLock->write(
               [&](const cursiv::CancellationToken&) {
                 report = registry(_shared).registerManifestEntries(_shared,
                                                                    entries);
               },
               cursiv::WorkspaceLock::WriteMode::Durable);
           return report;
         });
}

::lsp::InitializeResult XsmpLanguageServer::initialize(
    const ::lsp::InitializeParams& params) {
  _diagnostics.useContentRefresh(
      params.capabilities.workspace
      && params.capabilities.workspace->textDocumentContent);

  auto result = cursiv::DefaultLanguageServer::initialize(params);
  // A client reports folder changes only to a server that asks for them.
  result.capabilities.workspace->workspaceFolders->changeNotifications = true;
  return result;
}

void XsmpLanguageServer::didChangeWorkspaceFolders(
    const ::lsp::DidChangeWorkspaceFoldersParams& params) {
  _shared.workspaceLock->write(
      [&](const cursiv::CancellationToken& token) {
        std::vector<cursiv::WorkspaceFolder> folders(
            _shared.workspaceManager->workspaceFolders().begin(),
            _shared.workspaceManager->workspaceFolders().end());

        for (const auto& removed : params.event.removed)
          std::erase_if(folders, [&](const auto& folder) {
            return folder.uri == removed.uri.toString();
          });
        for (const auto& added : params.event.added)
          if (std::ranges::none_of(folders, [&](const auto& folder) {
                return folder.uri == added.uri.toString();
              }))
            folders.push_back(
                {.uri = added.uri.toString(), .name = added.name});

        // The XSMP workspace manager stages folder removals and mirror changes
        // before a single build, preserving buffers and the workspace lock
        // boundary.
        _shared.projectManager->contributionsChanged();
        _shared.workspaceManager->initializeWorkspace(folders, token);
      },
      cursiv::WorkspaceLock::WriteMode::Durable);
}

// The document builder refreshes the mirrors on any change that can move them.
// A deleted SMP file is the exception: no document stands for it, so the
// deletion would reach the builder empty. Its mirror does.
void XsmpLanguageServer::didChangeWatchedFiles(
    const ::lsp::DidChangeWatchedFilesParams& params) {
  auto expanded = params;
  for (const auto& change : params.changes)
    if (const auto uri = change.uri.toString();
        change.type == ::lsp::FileChangeType::Deleted && isSmpFile(uri))
      expanded.changes.push_back(
          {.uri = ::lsp::Uri::parse(smpMirrorUri(uri)), .type = change.type});

  cursiv::DefaultLanguageServer::didChangeWatchedFiles(expanded);
}

}  // namespace xsmp
