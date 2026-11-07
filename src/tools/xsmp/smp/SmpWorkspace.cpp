#include <algorithm>
#include <cursiv/observability/ObservabilitySink.hpp>
#include <cursiv/services/ServiceRegistry.hpp>
#include <cursiv/utils/UriUtils.hpp>
#include <cursiv/workspace/DefaultWorkspaceManager.hpp>
#include <cursiv/workspace/DocumentBuilder.hpp>
#include <cursiv/workspace/DocumentFactory.hpp>
#include <cursiv/workspace/Documents.hpp>
#include <cursiv/workspace/FileSystemProvider.hpp>
#include <cursiv/workspace/TextDocument.hpp>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <stdexcept>
#include <xsmp/ast.hpp>
#include <xsmp/smp/SmpService.hpp>
#include <xsmp/workspace/SourceUri.hpp>

// How SMP XML files enter a workspace: each one shows up as a read-only XSMP
// document, its mirror, rendered by the importer, unless a native XSMP file
// stands beside it. `SmpFileSystemProvider` serves the text of the mirrors,
// `SmpWorkspaceManager` decides which ones exist, and `SmpDocumentBuilder`
// refreshes them on the updates that can change them.

namespace xsmp {

bool ignoredDirectory(std::string_view name) {
  return name.starts_with('.') || name == "node_modules" || name == "out"
         || name == "smdl-gen";
}

namespace {

// True when `uri`, which `root` contains, has no ignored directory below
// `root`.
bool outsideIgnoredDirectories(std::string_view root, std::string_view uri) {
  auto relative = uri.substr(std::min(root.size() + 1, uri.size()));
  for (auto slash = relative.find('/'); slash != relative.npos;
       slash = relative.find('/')) {
    if (ignoredDirectory(cursiv::uri_file_name(relative.substr(0, slash))))
      return false;
    relative.remove_prefix(slash + 1);
  }
  return true;
}

// What the importer said per XML source, and the source roots of the projects.
// The file system provider and the workspace manager share it. Its mutex also
// guards their `ImportOptions`.
struct ImportErrors {
  mutable std::mutex mutex;
  std::map<std::string, std::string> messages;
  std::map<std::string, std::vector<std::string>> warnings;
  std::vector<std::string> roots;
  bool initialized = false;

  // Whether `uri` is in a source root, and neither hidden nor in an ignored
  // directory. Everything is before the first scan.
  bool inRoots(std::string_view uri) const {
    return !initialized || std::ranges::any_of(roots, [&](const auto& root) {
      return cursiv::contains_uri(root, uri)
             && !cursiv::uri_file_name(uri).starts_with('.')
             && outsideIgnoredDirectories(root, uri);
    });
  }

  // The outcome of one import, an empty message meaning success. Any warning of
  // that source is dropped.
  void set(std::string uri, std::string message) {
    std::scoped_lock lock(mutex);
    warnings.erase(uri);
    if (message.empty() || !inRoots(uri))
      messages.erase(uri);
    else
      messages.insert_or_assign(std::move(uri), std::move(message));
  }

  void setWarnings(std::string uri, std::vector<std::string> values) {
    std::scoped_lock lock(mutex);
    if (values.empty() || !inRoots(uri))
      warnings.erase(uri);
    else
      warnings.insert_or_assign(std::move(uri), std::move(values));
  }

  // Publishes a whole scan at once, `options` included.
  void replace(std::vector<std::string> sourceRoots,
               std::map<std::string, std::string> errors,
               std::map<std::string, std::vector<std::string>> sourceWarnings,
               ImportOptions& options, ImportOptions nextOptions) {
    std::scoped_lock lock(mutex);
    messages = std::move(errors);
    warnings = std::move(sourceWarnings);
    roots = std::move(sourceRoots);
    options = std::move(nextOptions);
    initialized = true;
  }
};

// The native extension that shadows an XML extension, empty for any other.
std::string_view nativeExtension(std::string_view xml) {
  for (const auto& [source, native] : smpExtensions)
    if (source == xml) return native;
  return {};
}

// The native file that would shadow the XML file `source`.
std::filesystem::path nativeSidecar(std::filesystem::path source) {
  source.replace_extension(nativeExtension(source.extension().string()));
  return source;
}

std::string shadowedWarning(const std::filesystem::path& source) {
  return "Ignoring '" + source.filename().string() + "' because '"
         + nativeSidecar(source).filename().string()
         + "' is present in the same source folder.";
}

// The XML file behind a mirror, `std::nullopt` for any other URI.
std::optional<std::filesystem::path> mirrorSourcePath(std::string_view uri) {
  if (!isSmpMirrorUri(uri)) return std::nullopt;
  const auto file = cursiv::file_uri_to_path(sourceDocumentUri(uri));
  return file ? std::optional<std::filesystem::path>(*file) : std::nullopt;
}

// The XML files under `root`, sorted, leaving out hidden files and ignored
// directories.
std::vector<std::filesystem::path> sourceFiles(
    const std::filesystem::path& root) {
  std::vector<std::filesystem::path> files;
  std::error_code error;
  if (!std::filesystem::is_directory(root, error)) return files;
  for (auto it = std::filesystem::recursive_directory_iterator(
           root, std::filesystem::directory_options::skip_permission_denied,
           error);
       it != std::filesystem::recursive_directory_iterator();
       it.increment(error)) {
    if (error) {
      error.clear();
      continue;
    }
    if (it->is_directory(error)) {
      const auto name = it->path().filename().string();
      if (ignoredDirectory(name)) it.disable_recursion_pending();
      continue;
    }
    if (!it->is_regular_file(error)) continue;
    if (it->path().filename().string().starts_with('.')) continue;
    if (nativeExtension(it->path().extension().string()).empty()) continue;
    files.push_back(it->path());
  }
  std::ranges::sort(files);
  return files;
}

// An XML source and the XSMP text rendered from it.
struct MirrorDocument {
  std::filesystem::path sourcePath;
  std::string sourceUri;
  std::string mirrorUri;
  std::string content;
  std::vector<std::string> warnings;
};

MirrorDocument renderMirror(const std::filesystem::path& source,
                            const ImportOptions& options) {
  auto imported = renderImportedDocument(source, options);
  const auto sourceUri = cursiv::path_to_file_uri(source.string());
  return {.sourcePath = source,
          .sourceUri = sourceUri,
          .mirrorUri = smpMirrorUri(sourceUri),
          .content = std::move(imported.content),
          .warnings = std::move(imported.warnings)};
}

// Serves the text of a mirror by importing its XML source, and records the
// outcome in `ImportErrors`. Any other URI goes to the wrapped provider.
class SmpFileSystemProvider final : public cursiv::FileSystemProvider {
 public:
  explicit SmpFileSystemProvider(
      std::unique_ptr<cursiv::FileSystemProvider> underlying,
      std::shared_ptr<ImportOptions> options,
      std::shared_ptr<ImportErrors> errors)
      : _underlying(std::move(underlying)),
        _options(std::move(options)),
        _errors(std::move(errors)) {}

  void record(std::string uri, std::string message) const {
    _errors->set(std::move(uri), std::move(message));
  }

  void recordWarnings(std::string uri, std::vector<std::string> values) const {
    _errors->setWarnings(std::move(uri), std::move(values));
  }

  // The current errors, forgetting those of a source that is gone or out of the
  // roots.
  std::vector<std::pair<std::string, std::string>> errors() const {
    std::scoped_lock lock(_errors->mutex);
    std::vector<std::pair<std::string, std::string>> result;
    for (auto it = _errors->messages.begin(); it != _errors->messages.end();) {
      const auto path = cursiv::file_uri_to_path(it->first);
      if (!path || !_errors->inRoots(it->first) || !mirrorExists(*path))
        it = _errors->messages.erase(it);
      else {
        result.emplace_back(*it);
        ++it;
      }
    }
    return result;
  }

  // The same for warnings. A source that is now shadowed only says so.
  std::vector<std::pair<std::string, std::string>> warnings() const {
    std::scoped_lock lock(_errors->mutex);
    std::vector<std::pair<std::string, std::string>> result;
    for (auto it = _errors->warnings.begin(); it != _errors->warnings.end();) {
      const auto path = cursiv::file_uri_to_path(it->first);
      std::error_code error;
      if (!path || !_errors->inRoots(it->first)
          || !std::filesystem::is_regular_file(*path, error)) {
        it = _errors->warnings.erase(it);
      } else {
        if (std::filesystem::exists(nativeSidecar(*path), error))
          result.emplace_back(it->first, shadowedWarning(*path));
        else
          for (const auto& warning : it->second)
            result.emplace_back(it->first, warning);
        ++it;
      }
    }
    return result;
  }

  cursiv::FileSystemNode stat(std::string_view uri) const override {
    if (auto source = mirrorSourcePath(uri))
      return {.isFile = available(*source),
              .isDirectory = false,
              .uri = std::string(uri)};
    return _underlying->stat(uri);
  }

  bool exists(std::string_view uri) const override {
    if (auto source = mirrorSourcePath(uri)) return available(*source);
    return _underlying->exists(uri);
  }

  std::string readFile(std::string_view uri) const override {
    if (auto source = mirrorSourcePath(uri)) {
      if (!available(*source))
        throw cursiv::FileSystemError(
            "SMP mirror is absent or shadowed by a native XSMP source: "
            + std::string(uri));
      try {
        ImportOptions options;
        {
          std::scoped_lock lock(_errors->mutex);
          options = *_options;
        }
        auto result = renderImportedDocument(*source, options);
        record(sourceDocumentUri(uri), {});
        recordWarnings(sourceDocumentUri(uri), std::move(result.warnings));
        return std::move(result.content);
      } catch (const std::exception& e) {
        record(sourceDocumentUri(uri), e.what());
        throw cursiv::FileSystemError(e.what());
      }
    }
    return _underlying->readFile(uri);
  }

  std::vector<std::uint8_t> readBinary(std::string_view uri) const override {
    if (mirrorSourcePath(uri)) {
      const auto value = readFile(uri);
      return {value.begin(), value.end()};
    }
    return _underlying->readBinary(uri);
  }

  // A mirror is never listed: `SmpWorkspaceManager` adds it to the workspace
  // itself.
  std::vector<cursiv::FileSystemNode> readDirectory(
      std::string_view uri) const override {
    return _underlying->readDirectory(uri);
  }

 private:
  // A mirror that is gone also loses its recorded error.
  bool available(const std::filesystem::path& source) const {
    if (mirrorExists(source)) return true;
    record(cursiv::path_to_file_uri(source.string()), {});
    return false;
  }

  // True when the XML file is there and no native file shadows it.
  static bool mirrorExists(const std::filesystem::path& source) {
    std::error_code error;
    return std::filesystem::is_regular_file(source, error)
           && !std::filesystem::exists(nativeSidecar(source), error);
  }

  std::unique_ptr<cursiv::FileSystemProvider> _underlying;
  std::shared_ptr<ImportOptions> _options;
  std::shared_ptr<ImportErrors> _errors;
};

// Scans like the default manager, and also keeps the mirrors in step with the
// `xsmp.project` files and the XML sources. A scan can run again with other
// folders. The folders and the promise of the base class are private, so this
// class has its own.
class SmpWorkspaceManager final : public cursiv::DefaultWorkspaceManager {
 public:
  SmpWorkspaceManager(cursiv::SharedCoreServices& shared,
                      std::shared_ptr<ImportOptions> options,
                      std::shared_ptr<ImportErrors> errors)
      : DefaultWorkspaceManager(shared),
        _options(std::move(options)),
        _errors(std::move(errors)) {}

  void initializeWorkspace(
      std::span<const cursiv::WorkspaceFolder> folders,
      const cursiv::CancellationToken& token = {}) override {
    _initializing = true;
    // The documents of the folders that left the workspace, unless they are
    // open.
    std::vector<std::string> closed;
    for (const auto& document : shared.documents->all())
      if (!document->isOpen && document->uri.starts_with("file:")
          && std::ranges::any_of(_folders,
                                 [&](const auto& folder) {
                                   return cursiv::contains_uri(folder.uri,
                                                               document->uri);
                                 })
          && !std::ranges::any_of(folders, [&](const auto& folder) {
               return cursiv::contains_uri(folder.uri, document->uri);
             }))
        closed.push_back(document->uri);

    _folders.assign(folders.begin(), folders.end());
    // A failure anywhere must still release whoever waits on `ready()`.
    try {
      std::vector<std::string> newlyRead;
      for (const auto& folder : folders)
        for (const auto& uri : searchFolder(getRootFolder(folder)))
          // An unreadable file stays out of the workspace.
          if (!shared.documents->hasDocument(uri)) {
            try {
              shared.documents->addDocument(
                  shared.documentFactory->fromUri(uri, token));
              newlyRead.push_back(uri);
            } catch (const cursiv::FileSystemError&) {
            }
          }

      // After the read, as `prepare` finds the projects among the documents.
      auto result = prepare({}, token);
      result.deleted.insert(result.deleted.end(), closed.begin(), closed.end());
      // As in the default manager, `ready()` resolves before the build.
      if (!_resolved) {
        _resolved = true;
        _readyPromise.set_value();
      }

      std::set<std::string> moved(newlyRead.begin(), newlyRead.end());
      moved.insert(result.changed.begin(), result.changed.end());
      const std::vector<std::string_view> changed(moved.begin(), moved.end());
      const std::vector<std::string_view> deleted(result.deleted.begin(),
                                                  result.deleted.end());

      // One update over what moved, with the options of a workspace scan.
      auto previous = shared.documentBuilder->updateBuildOptions();
      shared.documentBuilder->updateBuildOptions() = initialBuildOptions();
      try {
        shared.documentBuilder->update(changed, deleted, token);
      } catch (...) {
        shared.documentBuilder->updateBuildOptions() = previous;
        throw;
      }
      shared.documentBuilder->updateBuildOptions() = previous;
    } catch (...) {
      _initializing = false;
      if (!_resolved) {
        _resolved = true;
        _readyPromise.set_value();
      }
      throw;
    }
    _initializing = false;
  }

  std::span<const cursiv::WorkspaceFolder> workspaceFolders()
      const noexcept override {
    return _folders;
  }

  std::shared_future<void> ready() const override { return _ready; }

  // False during a scan, which refreshes the mirrors itself.
  bool canRefresh() const { return !_initializing; }

  // Whether an update of `uri` can change the mirrors: a project, a mirror, an
  // XML file in a source root or named by a link, or a native file that starts
  // or stops shadowing its XML file. Anything else, an edit of a native file or
  // an XML file that generation writes, costs no walk of the source roots.
  bool movesMirrors(std::string_view uri) const {
    if (cursiv::uri_file_name(uri) == "xsmp.project" || isSmpMirrorUri(uri))
      return true;
    const auto extension = cursiv::uri_extension(uri);
    for (const auto& [xml, native] : smpExtensions) {
      if (extension == xml) {
        if (const auto path = cursiv::file_uri_to_path(uri);
            path
            && _references.contains(
                std::filesystem::path(*path).filename().string()))
          return true;
        std::scoped_lock lock(_errors->mutex);
        return _errors->inRoots(uri);
      }
      if (extension != native) continue;
      const auto path = cursiv::file_uri_to_path(uri);
      if (!path) return false;
      auto source = std::filesystem::path(*path);
      source.replace_extension(xml);
      std::error_code error;
      return std::filesystem::is_regular_file(source, error)
             && std::filesystem::exists(*path, error)
                    == _active.contains(smpMirrorUri(
                        cursiv::path_to_file_uri(source.string())));
    }
    return false;
  }

  // A native document follows the walk of the default manager, over the folders
  // of this class. A mirror needs its XML source in a source root.
  bool isIncluded(const cursiv::FileSystemNode& entry) const override {
    if (!isSmpMirrorUri(entry.uri)) {
      auto held = false;
      for (const auto& folder : _folders) {
        const auto root = cursiv::normalize_uri(folder.uri);
        if (!cursiv::contains_uri(root, entry.uri)) continue;
        held = true;
        auto step = entry;
        auto included = true;
        while (!step.uri.empty() && !cursiv::contains_uri(step.uri, root)) {
          if (!shouldIncludeEntry(step)) {
            included = false;
            break;
          }
          while (step.uri.ends_with('/')) step.uri.pop_back();
          const auto slash = step.uri.rfind('/');
          step = {
              .isFile = false,
              .isDirectory = true,
              .uri = step.uri.substr(0, slash == step.uri.npos ? 0 : slash)};
        }
        if (included) return true;
      }
      return !held && shouldIncludeEntry(entry);
    }
    if (!DefaultWorkspaceManager::shouldIncludeEntry(entry)) return false;
    const auto source = sourceDocumentUri(entry.uri);
    std::scoped_lock lock(_errors->mutex);
    if (!_errors->initialized) return false;
    return std::ranges::any_of(_errors->roots, [&](const auto& root) {
      return cursiv::contains_uri(root, source)
             && outsideIgnoredDirectories(root, source);
    });
  }

  // Drains what `prepare` recorded, with the latest outcome of each URI.
  MirrorRefreshResult takeChanges() {
    MirrorRefreshResult result;
    for (const auto& [uri, changed] : _pending)
      (changed ? result.changed : result.deleted).push_back(uri);
    _pending.clear();
    return result;
  }

  using ProjectOverrides =
      std::map<std::string, std::shared_ptr<cursiv::Document>>;

  // Refreshes the mirrors before the builder applies an update, which has not
  // reached the project documents yet: their new state comes in as overrides,
  // with a null document for a deleted project.
  MirrorRefreshResult prepareUpdate(std::span<const std::string_view> changed,
                                    std::span<const std::string_view> deleted,
                                    const cursiv::CancellationToken& token) {
    ProjectOverrides overrides;
    for (auto uri : deleted)
      if (cursiv::uri_file_name(uri) == "xsmp.project")
        overrides.emplace(uri, nullptr);
    for (auto uri : changed)
      if (cursiv::uri_file_name(uri) == "xsmp.project") {
        const auto previous = shared.documents->getDocument(uri);
        auto text =
            previous ? std::string(previous->textDocument().getText()) : "";
        // An open project keeps the text of the editor.
        if (!previous || !previous->isOpen) {
          try {
            text = shared.fileSystemProvider->readFile(uri);
          } catch (const cursiv::FileSystemError&) {
          }
        }
        overrides.insert_or_assign(
            std::string(uri),
            shared.documentFactory->fromString(std::move(text), uri, token));
      }
    return prepare(overrides, token);
  }

 private:
  // Recomputes the mirrors from the projects: the source roots, then the XML
  // files, then a render of each. Returns the difference with the last time,
  // and also records it for `takeChanges`.
  MirrorRefreshResult prepare(const ProjectOverrides& overrides,
                              const cursiv::CancellationToken& token) {
    // `roots` hold the XML sources. `searchRoots` add the workspace folders,
    // where a link can find another document.
    std::set<std::filesystem::path> roots, searchRoots, sources;
    for (const auto& folder : workspaceFolders())
      if (const auto path = cursiv::file_uri_to_path(folder.uri))
        searchRoots.emplace(*path);

    std::vector<std::shared_ptr<cursiv::Document>> projects;
    for (const auto& document : shared.documents->all())
      if (cursiv::uri_file_name(document->uri) == "xsmp.project"
          && !overrides.contains(document->uri))
        projects.push_back(document);
    for (const auto& [uri, document] : overrides)
      if (document) projects.push_back(document);

    for (const auto& document : projects) {
      const auto* project =
          cursiv::ast_cast<Project>(document->parseResult.value);
      if (!project
          || !std::ranges::any_of(workspaceFolders(), [&](const auto& folder) {
               return cursiv::contains_uri(folder.uri, document->uri);
             }))
        continue;
      const auto file = cursiv::file_uri_to_path(document->uri);
      if (!file) continue;
      const auto directory = std::filesystem::path(*file).parent_path();
      const auto directoryUri = cursiv::path_to_file_uri(directory.string());
      for (const auto* element : project->elements)
        if (const auto* source = cursiv::ast_cast<Source>(element);
            source && !source->path.empty()) {
          std::string path(source->path);
          std::ranges::replace(path, '\\', '/');
          const auto root = (directory / path).lexically_normal();
          // A source outside the folder of its project is ignored.
          if (cursiv::contains_uri(directoryUri,
                                   cursiv::path_to_file_uri(root.string())))
            roots.insert(root);
        }
    }

    std::vector<std::string> rootUris;
    for (const auto& root : roots) {
      auto uri = cursiv::normalize_uri(cursiv::path_to_file_uri(root.string()));
      while (uri.ends_with('/') && !uri.ends_with(":///")) uri.pop_back();
      rootUris.push_back(std::move(uri));
      searchRoots.insert(root);
      for (auto source : sourceFiles(root)) sources.insert(std::move(source));
    }

    ImportOptions nextOptions;
    nextOptions.searchRoots.assign(searchRoots.begin(), searchRoots.end());
    nextOptions.documents = shared.documents.get();
    std::set<std::string> nextReferences;
    nextOptions.references = &nextReferences;
    std::map<std::string, std::string> nextErrors;
    std::map<std::string, std::vector<std::string>> nextWarnings;
    std::vector<MirrorDocument> mirrors;

    for (const auto& source : sources) {
      cursiv::throw_if_cancelled(token);
      std::error_code error;
      if (std::filesystem::exists(nativeSidecar(source), error)) {
        nextWarnings.emplace(cursiv::path_to_file_uri(source.string()),
                             std::vector<std::string>{shadowedWarning(source)});
        continue;
      }
      try {
        auto mirror = renderMirror(source, nextOptions);
        if (!mirror.warnings.empty())
          nextWarnings.emplace(mirror.sourceUri, mirror.warnings);
        mirrors.push_back(std::move(mirror));
      } catch (const cursiv::OperationCancelled&) {
        throw;
      } catch (const std::exception& error) {
        nextErrors.emplace(cursiv::path_to_file_uri(source.string()),
                           error.what());
      }
    }

    // Nothing is published until every source is rendered.
    cursiv::throw_if_cancelled(token);
    nextOptions.references = nullptr;
    _references = std::move(nextReferences);
    _errors->replace(std::move(rootUris), std::move(nextErrors),
                     std::move(nextWarnings), *_options,
                     std::move(nextOptions));

    MirrorRefreshResult result;
    std::set<std::string> nextActive;
    for (auto& mirror : mirrors) {
      nextActive.insert(mirror.mirrorUri);
      const auto previous = shared.documents->getDocument(mirror.mirrorUri);
      if (!previous || !_active.contains(mirror.mirrorUri)
          || previous->textDocument().getText() != mirror.content) {
        result.changed.push_back(mirror.mirrorUri);
        _pending.insert_or_assign(mirror.mirrorUri, true);
        // A new mirror enters here. A known one is read again by the update
        // that follows.
        if (!previous)
          shared.documents->addDocument(shared.documentFactory->fromString(
              std::move(mirror.content), mirror.mirrorUri, token));
      }
    }

    for (const auto& old : _active)
      if (!nextActive.contains(old)) {
        result.deleted.push_back(old);
        _pending.insert_or_assign(old, false);
      }
    _active = std::move(nextActive);
    return result;
  }

  std::shared_ptr<ImportOptions> _options;
  std::shared_ptr<ImportErrors> _errors;
  // The mirror URIs of the last `prepare`.
  std::set<std::string> _active;
  // The file names that the links of the last `prepare` named, found or not.
  std::set<std::string> _references;
  // Per URI, whether it changed or was deleted, until `takeChanges`.
  std::map<std::string, bool> _pending;
  bool _initializing = false;
  std::vector<cursiv::WorkspaceFolder> _folders;
  std::promise<void> _readyPromise;
  std::shared_future<void> _ready = _readyPromise.get_future().share();
  bool _resolved = false;
};

// Forwards everything to the wrapped builder, and refreshes the mirrors before
// an update that touches a project, an XML source or its native sidecar.
class SmpDocumentBuilder final : public cursiv::DocumentBuilder {
 public:
  SmpDocumentBuilder(cursiv::SharedCoreServices& shared,
                     std::unique_ptr<cursiv::DocumentBuilder> underlying)
      : _shared(shared), _underlying(std::move(underlying)) {}

  void addBuildPhaseListener(cursiv::BuildPhaseListener& x) override {
    _underlying->addBuildPhaseListener(x);
  }

  void removeBuildPhaseListener(cursiv::BuildPhaseListener& x) override {
    _underlying->removeBuildPhaseListener(x);
  }

  void addDocumentPhaseListener(cursiv::DocumentPhaseListener& x) override {
    _underlying->addDocumentPhaseListener(x);
  }

  void removeDocumentPhaseListener(cursiv::DocumentPhaseListener& x) override {
    _underlying->removeDocumentPhaseListener(x);
  }

  void addDocumentUpdateListener(cursiv::DocumentUpdateListener& x) override {
    _underlying->addDocumentUpdateListener(x);
  }

  void removeDocumentUpdateListener(
      cursiv::DocumentUpdateListener& x) override {
    _underlying->removeDocumentUpdateListener(x);
  }

  void build(std::span<cursiv::Document* const> documents,
             const cursiv::BuildOptions& options = {},
             const cursiv::CancellationToken& token = {}) const override {
    _underlying->build(documents, options, token);
  }

  void update(std::span<const std::string_view> changed,
              std::span<const std::string_view> deleted,
              const cursiv::CancellationToken& token = {}) const override {
    auto* manager =
        dynamic_cast<SmpWorkspaceManager*>(_shared.workspaceManager.get());
    const auto relevant = [&](auto uris) {
      return std::ranges::any_of(uris, [&](std::string_view uri) {
        return manager->movesMirrors(uri);
      });
    };

    std::set<std::string> nextChanged(changed.begin(), changed.end());
    std::set<std::string> nextDeleted(deleted.begin(), deleted.end());
    if (manager && manager->canRefresh()
        && (relevant(changed) || relevant(deleted))) {
      auto result = manager->prepareUpdate(changed, deleted, token);
      nextChanged.insert(result.changed.begin(), result.changed.end());
      nextDeleted.insert(result.deleted.begin(), result.deleted.end());
      // A mirror that the scan would no longer reach drops out.
      std::erase_if(nextChanged, [&](const auto& uri) {
        return isSmpMirrorUri(uri)
               && !manager->isIncluded(_shared.fileSystemProvider->stat(uri));
      });
    }

    // A deletion wins over a change of the same URI.
    for (const auto& uri : nextDeleted) nextChanged.erase(uri);
    const std::vector<std::string_view> changes(nextChanged.begin(),
                                                nextChanged.end());
    const std::vector<std::string_view> deletions(nextDeleted.begin(),
                                                  nextDeleted.end());
    _underlying->update(changes, deletions, token);
  }

  cursiv::BuildOptions& updateBuildOptions() noexcept override {
    return _underlying->updateBuildOptions();
  }

  const cursiv::BuildOptions& updateBuildOptions() const noexcept override {
    return _underlying->updateBuildOptions();
  }

  void resetToState(cursiv::Document& document,
                    cursiv::DocumentState state) const override {
    _underlying->resetToState(document, state);
  }

  cursiv::WaitOutcome waitUntil(
      cursiv::DocumentState state,
      const cursiv::CancellationToken& token = {}) const override {
    return _underlying->waitUntil(state, token);
  }

  cursiv::WaitOutcome waitUntil(
      cursiv::DocumentState state, cursiv::DocumentId id,
      const cursiv::CancellationToken& token = {}) const override {
    return _underlying->waitUntil(state, id, token);
  }

  cursiv::DocumentState currentState() const noexcept override {
    return _underlying->currentState();
  }

 private:
  cursiv::SharedCoreServices& _shared;
  std::unique_ptr<cursiv::DocumentBuilder> _underlying;
};

}  // namespace

void installSmpWorkspaceManager(cursiv::SharedCoreServices& shared) {
  // Installing twice would wrap the services twice.
  if (dynamic_cast<SmpFileSystemProvider*>(shared.fileSystemProvider.get()))
    return;
  auto options = std::make_shared<ImportOptions>();
  auto errors = std::make_shared<ImportErrors>();
  shared.fileSystemProvider = std::make_unique<SmpFileSystemProvider>(
      std::move(shared.fileSystemProvider), options, errors);
  shared.workspaceManager =
      std::make_unique<SmpWorkspaceManager>(shared, options, errors);
  shared.documentBuilder = std::make_unique<SmpDocumentBuilder>(
      shared, std::move(shared.documentBuilder));
}

MirrorRefreshResult takeMirrorChanges(cursiv::SharedCoreServices& shared) {
  if (auto* manager =
          dynamic_cast<SmpWorkspaceManager*>(shared.workspaceManager.get()))
    return manager->takeChanges();
  return {};
}

std::vector<std::pair<std::string, std::string>> sourceErrors(
    const cursiv::SharedCoreServices& shared) {
  const auto* provider = dynamic_cast<const SmpFileSystemProvider*>(
      shared.fileSystemProvider.get());
  return provider ? provider->errors()
                  : std::vector<std::pair<std::string, std::string>>{};
}

std::vector<std::pair<std::string, std::string>> sourceWarnings(
    const cursiv::SharedCoreServices& shared) {
  const auto* provider = dynamic_cast<const SmpFileSystemProvider*>(
      shared.fileSystemProvider.get());
  return provider ? provider->warnings()
                  : std::vector<std::pair<std::string, std::string>>{};
}

}  // namespace xsmp
