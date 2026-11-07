#include <algorithm>
#include <cursiv/services/SharedCoreServices.hpp>
#include <cursiv/syntax-tree/DocumentReferences.hpp>
#include <cursiv/utils/UriUtils.hpp>
#include <cursiv/workspace/DefaultIndexManager.hpp>
#include <cursiv/workspace/Document.hpp>
#include <cursiv/workspace/DocumentBuilder.hpp>
#include <cursiv/workspace/Documents.hpp>
#include <functional>
#include <mutex>
#include <span>
#include <string>
#include <unordered_map>
#include <xsmp/SharedCoreServices.hpp>
#include <xsmp/utils/TreeUtils.hpp>
#include <xsmp/workspace/ProjectManager.hpp>
#include <xsmp/workspace/SourceUri.hpp>

namespace xsmp {

namespace {

// True for the URI of an `xsmp.project`, `.xsmpprofile` or `.xsmptool`
// document: the three kinds whose edits can change a catalogue's visible
// document set through `dependency` and `source`, even though a catalogue never
// holds a direct AST reference to any of them.
bool isProjectGraphUri(std::string_view uri) {
  const auto extension = cursiv::uri_extension(uri);
  return cursiv::uri_file_name(uri) == "xsmp.project"
         || extension == ".xsmpprofile" || extension == ".xsmptool";
}

// The name tells for a document with no tree yet.
bool isProjectGraphDocument(const cursiv::Document& document) {
  const auto* root = document.parseResult.value;
  return isProjectGraphUri(document.uri)
         || cursiv::ast_cast<const Project>(root) != nullptr
         || cursiv::ast_cast<const Profile>(root) != nullptr
         || cursiv::ast_cast<const Tool>(root) != nullptr;
}

// An edit revalidates the documents that read the edited one:
// - through their references or a diagnostic that points into it, at any
//   depth, as an inherited member or a cycle of catalogues does;
// - without a reference, as the uniqueness checks do: whoever sees it when an
//   exported name changes, and every document when something the workspace
//   keeps unique changes, a document, service or project name or a `@uuid`;
// - through the project graph, which says what every modeling document sees. A
//   document may see less after the edit, so its visible set cannot tell.
// A built-in reads only built-ins, which do not change once loaded.
class XsmpIndexManager final : public cursiv::DefaultIndexManager {
 public:
  XsmpIndexManager(cursiv::SharedCoreServices& shared,
                   const ProjectManager& manager)
      : DefaultIndexManager(shared), _manager(manager) {}

  void updateContent(const cursiv::Document& document,
                     const cursiv::CancellationToken& token) override {
    DefaultIndexManager::updateContent(document, token);
    const std::scoped_lock lock(_mutex);
    _impact.reset();
  }

  void updateReferences(const cursiv::Document& document,
                        const cursiv::CancellationToken& token) override {
    DefaultIndexManager::updateReferences(document, token);
    std::unordered_set<cursiv::DocumentId> targets;
    if (const auto* references = document.parseResult.references.get())
      references->visitResolved(document.parseResult.cst(),
                                [&](cursiv::TextOffset, cursiv::TextOffset,
                                    const cursiv::AstNode& target, bool) {
                                  if (const auto* home = target.getDocument();
                                      home && home != &document)
                                    targets.insert(home->id);
                                });
    const std::scoped_lock lock(_mutex);
    _targets.insert_or_assign(document.id, std::move(targets));
    _impact.reset();
  }

  bool remove(cursiv::DocumentId document) override {
    {
      const std::scoped_lock lock(_mutex);
      _targets.erase(document);
      _published.erase(document);
      _impact.reset();
    }
    return DefaultIndexManager::remove(document);
  }

  bool isAffected(
      const cursiv::Document& document,
      const std::unordered_set<cursiv::DocumentId>& changed) const override {
    if (DefaultIndexManager::isAffected(document, changed)) return true;
    if (changed.empty()) return false;
    const auto impact = impactOf(changed);
    if (isBuiltinUri(document.uri)) return false;
    if (impact->unique || impact->readers.contains(document.id)) return true;
    if (impact->projectGraph
        && cursiv::ast_cast<const Document>(document.parseResult.value))
      return true;
    // A broken reference may find its target in anything that changed.
    const auto broken = document.hasLinkingErrors();
    if (!broken && impact->published.empty()) return false;
    const auto visible = _manager.getVisibleDocumentIds(document);
    return std::ranges::any_of(
        broken ? changed : impact->published,
        [&](const auto& id) { return visible->contains(id); });
  }

 private:
  // What one set of changed documents reaches. The builder asks for every
  // document in turn, so it is computed once.
  struct Impact {
    std::unordered_set<cursiv::DocumentId> changed;
    // Who reads one of them, at any depth.
    std::unordered_set<cursiv::DocumentId> readers;
    // Those whose exported names changed, which reaches who sees them.
    std::unordered_set<cursiv::DocumentId> published;
    // Whether what the workspace keeps unique changed, which reaches everyone.
    bool unique = false;
    // Whether one of them is a project, a profile or a tool.
    bool projectGraph = false;
  };

  // Hashes of what others read of a document without a reference.
  struct Published {
    std::size_t names = 0;
    std::size_t unique = 0;
  };

  // What others read of each of `documents`, in one pass over the index.
  // Searching the text for `@uuid` costs less than reading the tag of each type,
  // and a false match only revalidates more.
  std::unordered_map<cursiv::DocumentId, Published> publishedOf(
      std::span<const cursiv::DocumentId> documents) const {
    std::unordered_map<cursiv::DocumentId, Published> result;
    const auto add = [](std::size_t& hash, std::size_t value) {
      hash = hash * 31 + value;
    };
    (void)visitElements(
        nullptr, documents, [&](const cursiv::AstNodeDescription& element) {
          const auto unique =
              element.type
              && (element.type->derivesFrom(&cursiv::ast_type_tag<Document>)
                  || element.type->derivesFrom(&cursiv::ast_type_tag<Service>)
                  || element.type->derivesFrom(&cursiv::ast_type_tag<Project>));
          auto& published = result[element.documentId];
          auto& hash = unique ? published.unique : published.names;
          add(hash, std::hash<std::string_view>{}(element.name));
          add(hash, std::hash<const void*>{}(element.type));
          return true;
        });
    constexpr std::string_view tag = "@uuid";
    for (const auto id : documents) {
      auto& published = result[id];
      const auto document = shared.documents->getDocument(id);
      const std::string_view text = document->textDocument().getText();
      for (auto at = text.find(tag); at != std::string_view::npos;
           at = text.find(tag, at + tag.size()))
        add(published.unique,
            std::hash<std::string_view>{}(
                text.substr(at, text.find_first_of("*\n", at) - at)));
    }
    return result;
  }

  std::shared_ptr<const Impact> impactOf(
      const std::unordered_set<cursiv::DocumentId>& changed) const {
    const std::scoped_lock lock(_mutex);
    if (_impact && _impact->changed == changed) return _impact;
    auto impact = std::make_shared<Impact>();
    impact->changed = changed;

    std::unordered_map<cursiv::DocumentId, std::vector<cursiv::DocumentId>>
        incoming;
    for (const auto& [reader, targets] : _targets)
      for (const auto target : targets) incoming[target].push_back(reader);
    for (const auto& reader : shared.documents->all())
      for (const auto& diagnostic : reader->diagnostics)
        for (const auto& related : diagnostic.relatedInformation)
          if (const auto target = shared.documents->getDocument(related.uri);
              target && target != reader)
            incoming[target->id].push_back(reader->id);
    std::vector<cursiv::DocumentId> pending(changed.begin(), changed.end());
    while (!pending.empty()) {
      const auto id = pending.back();
      pending.pop_back();
      if (const auto at = incoming.find(id); at != incoming.end())
        for (const auto reader : at->second)
          if (impact->readers.insert(reader).second) pending.push_back(reader);
    }

    std::vector<cursiv::DocumentId> indexed;
    for (const auto id : changed) {
      const auto document = shared.documents->getDocument(id);
      if (!document) continue;
      if (isProjectGraphDocument(*document)) {
        impact->projectGraph = true;
        impact->published.insert(id);
      }
      // A document not indexed yet says nothing until it is.
      if (document->state.load() >= cursiv::DocumentState::IndexedContent)
        indexed.push_back(id);
    }
    // No document at all would read as every document.
    if (!indexed.empty())
      for (const auto& [id, published] : publishedOf(indexed)) {
        auto& known = _published[id];
        if (known.names != published.names) impact->published.insert(id);
        impact->unique = impact->unique || known.unique != published.unique;
        known = published;
      }
    _impact = impact;
    return impact;
  }

  const ProjectManager& _manager;
  mutable std::mutex _mutex;
  // The documents each one last referred to. Kept through a reset, so that the
  // order in which documents are reset does not cut a path between two others.
  std::unordered_map<cursiv::DocumentId, std::unordered_set<cursiv::DocumentId>>
      _targets;
  mutable std::unordered_map<cursiv::DocumentId, Published> _published;
  mutable std::shared_ptr<const Impact> _impact;
};

}  // namespace

std::string_view builtinStandard(std::string_view uri) {
  for (const auto standard : kSmpStandards) {
    if (uri.find(std::string("%40").append(standard))
        != std::string_view::npos) {
      return standard;
    }
  }
  return {};
}

ProjectManager::ProjectManager(cursiv::SharedCoreServices& shared)
    : cursiv::DefaultSharedCoreService(shared),
      _builder(shared.documentBuilder.get()) {
  // Replaces the default index manager, so project visibility drives relinking.
  shared.indexManager = std::make_unique<XsmpIndexManager>(shared, *this);
  _builder->addBuildPhaseListener(*this);
  _builder->addDocumentUpdateListener(*this);
}

ProjectManager::~ProjectManager() {
  if (!_builder) return;
  _builder->removeBuildPhaseListener(*this);
  _builder->removeDocumentUpdateListener(*this);
}

void ProjectManager::clearCaches() {
  _projects.clear();
  _projectCache.clear();
  _visibleCache.clear();
}

void ProjectManager::contributionsChanged() {
  clearCaches();
  for (const auto& document : shared.documents->all())
    if ((cursiv::ast_cast<const Document>(document->parseResult.value)
         || isProjectGraphDocument(*document))
        && document->state.load() >= cursiv::DocumentState::ComputedScopes)
      shared.documentBuilder->resetToState(
          *document, cursiv::DocumentState::ComputedScopes);
}

void ProjectManager::onBuildPhase(cursiv::DocumentState reached,
                                  std::span<cursiv::Document* const>,
                                  const cursiv::CancellationToken&) {
  if (reached != cursiv::DocumentState::IndexedContent || !_clearWhenParsed)
    return;
  _clearWhenParsed = false;
  clearCaches();
}

void ProjectManager::onUpdate(std::span<const std::string_view> changed,
                              std::span<const std::string_view> deleted) {
  const auto documentCount = shared.documents->all().size();
  if (documentCount != _documentCount || !deleted.empty()
      || std::ranges::any_of(changed, isProjectGraphUri)) {
    _documentCount = documentCount;
    _clearWhenParsed = true;
    clearCaches();
  }
  if (deleted.empty()) return;
  // What a built-in sees does not depend on the projects.
  for (const auto& document : shared.documents->all())
    if (cursiv::ast_cast<const Document>(document->parseResult.value)
        && !isBuiltinUri(document->uri))
      shared.documentBuilder->resetToState(
          *document, cursiv::DocumentState::ComputedScopes);
}

std::vector<const Project*> ProjectManager::getProjects() const {
  return _projects.get({}, [this] {
    std::vector<const Project*> projects;
    for (const auto& document : shared.documents->all())
      if (const auto* project =
              cursiv::ast_cast<const Project>(document->parseResult.value))
        projects.push_back(project);
    return projects;
  });
}

const Project* ProjectManager::getProjectByName(std::string_view name) const {
  for (const auto* project : getProjects()) {
    if (project->name == name) {
      return project;
    }
  }
  return nullptr;
}

const Project* ProjectManager::getProject(
    const cursiv::Document& document) const {
  return _projectCache.get(
      document.id, [this, &document] { return doGetProject(document); });
}

const Project* ProjectManager::doGetProject(
    const cursiv::Document& document) const {
  const Project* bestProject = nullptr;
  std::size_t bestDirectoryLength = 0;
  const auto sourceUri = sourceDocumentUri(document.uri);
  for (const auto* project : getProjects()) {
    // Both are normal URIs, so that a folder holding a document is its prefix.
    const auto projectDirUri = dirnameOfUri(documentOf(*project).uri);
    if (!sourceUri.starts_with(projectDirUri)
        || !cursiv::contains_uri(projectDirUri, sourceUri)) {
      continue;
    }
    for (const auto& element : project->elements) {
      const auto* source = cursiv::ast_cast<const Source>(element);
      if (source == nullptr || source->path.empty()) {
        continue;
      }
      const auto folderUri = resolveSourceUri(projectDirUri, source->path);
      if (cursiv::contains_uri(folderUri, sourceUri)) {
        // The deepest directory wins: a nested project owns its own sources.
        if (!bestProject || projectDirUri.size() > bestDirectoryLength) {
          bestProject = project;
          bestDirectoryLength = projectDirUri.size();
        }
        break;
      }
    }
  }
  return bestProject;
}

std::unordered_set<const Project*> ProjectManager::getDependencies(
    const Project& project) const {
  std::unordered_set<const Project*> dependencies{&project};
  collectDependencies(project, dependencies);
  return dependencies;
}

void ProjectManager::collectDependencies(
    const Project& project, std::unordered_set<const Project*>& out) const {
  for (const auto& element : project.elements) {
    const auto* dependency = cursiv::ast_cast<const Dependency>(element);
    if (dependency == nullptr) {
      continue;
    }

    // Visibility is asked during linking, when the references of this project
    // may not resolve yet: until it is linked, a dependency is found by name.
    const auto* document = project.getDocument();
    const auto* target =
        document && document->state.load() < cursiv::DocumentState::Linked
            ? getProjectByName(dependency->project.refText)
            : dependency->project.get();
    if (target != nullptr && out.insert(target).second) {
      collectDependencies(*target, out);
    }
  }
}

std::shared_ptr<const std::unordered_set<cursiv::DocumentId>>
ProjectManager::getVisibleDocumentIds(const cursiv::Document& document) const {
  // The documents of a project share one set.
  const auto* project = getProject(document);
  const void* key = project ? static_cast<const void*>(project) : &document;
  return _visibleCache.get(key, [this, &document, project] {
    std::unordered_set<cursiv::DocumentId> ids;
    if (project) {
      ids = computeVisibleDocumentIds(*project);
    } else if (isBuiltinUri(document.uri)) {
      ids = computeBuiltinVisibleDocumentIds(document);
    } else {
      ids = computeStandaloneVisibleDocumentIds(document);
    }
    return std::shared_ptr<const std::unordered_set<cursiv::DocumentId>>(
        std::make_shared<std::unordered_set<cursiv::DocumentId>>(
            std::move(ids)));
  });
}

std::unordered_set<cursiv::DocumentId>
ProjectManager::computeVisibleDocumentIds(const Project& project) const {
  std::unordered_set<cursiv::DocumentId> ids;
  ids.insert(documentOf(project).id);

  // The profiles and tools this project activates, by their canonical names.
  const auto* xsmpShared = dynamic_cast<const XsmpSharedCoreServices*>(&shared);
  std::unordered_set<std::string> activeContributions;
  if (xsmpShared) {
    for (const auto* element : project.elements) {
      std::string name;
      if (const auto* profile =
              cursiv::ast_cast<const ProfileReference>(element))
        name = profile->profile.refText;
      if (const auto* tool = cursiv::ast_cast<const ToolReference>(element))
        name = tool->tool.refText;
      if (name.empty()) continue;
      if (const auto alias = xsmpShared->contributionAliases.find(name);
          alias != xsmpShared->contributionAliases.end())
        name = alias->second;
      activeContributions.insert(std::move(name));
    }
  }

  // The closure holds the project itself, so its own documents are counted too.
  const auto dependencies = getDependencies(project);
  for (const auto* dependency : dependencies)
    ids.insert(documentOf(*dependency).id);

  const auto standard = project.standard;
  for (const auto& candidateDoc : shared.documents->all()) {
    if (const auto* owner = getProject(*candidateDoc)) {
      if (dependencies.contains(owner)) ids.insert(candidateDoc->id);
      continue;
    }

    // A contribution payload is visible only when this very project activates
    // its profile or tool. The other built-ins follow the standard.
    if (isBuiltinUri(candidateDoc->uri)) {
      if (xsmpShared) {
        const auto uri = std::string(candidateDoc->uri);
        const auto owner = xsmpShared->contributionPayloadOwners.find(uri);
        const auto contributors =
            xsmpShared->contributionPayloadContributors.find(uri);
        if (owner != xsmpShared->contributionPayloadOwners.end()
            || contributors
                   != xsmpShared->contributionPayloadContributors.end()) {
          const auto activeOwner =
              owner != xsmpShared->contributionPayloadOwners.end()
              && activeContributions.contains(owner->second);
          const auto activeContributor =
              contributors != xsmpShared->contributionPayloadContributors.end()
              && std::ranges::any_of(contributors->second, [&](const auto& id) {
                   return activeContributions.contains(id);
                 });
          if (activeOwner || activeContributor) ids.insert(candidateDoc->id);
          continue;
        }
      }
      const auto candidateStandard = builtinStandard(candidateDoc->uri);
      if (candidateStandard.empty() || candidateStandard == standard) {
        ids.insert(candidateDoc->id);
      }
    }
  }
  return ids;
}

std::unordered_set<cursiv::DocumentId>
ProjectManager::collectCoreBuiltinDocumentIds(std::string_view standard) const {
  std::unordered_set<cursiv::DocumentId> ids;
  for (const auto& candidateDoc : shared.documents->all()) {
    if (!isBuiltinUri(candidateDoc->uri)) {
      continue;
    }
    if (const auto* xsmpShared =
            dynamic_cast<const XsmpSharedCoreServices*>(&shared);
        xsmpShared
        && (xsmpShared->contributionPayloadOwners.contains(
                std::string(candidateDoc->uri))
            || xsmpShared->contributionPayloadContributors.contains(
                std::string(candidateDoc->uri))))
      continue;
    const auto candidateStandard = builtinStandard(candidateDoc->uri);
    if (standard.empty()) {
      if (candidateStandard.empty()) {
        ids.insert(candidateDoc->id);
      }
      continue;
    }
    if (candidateStandard.empty() || candidateStandard == standard) {
      ids.insert(candidateDoc->id);
    }
  }
  return ids;
}

std::unordered_set<cursiv::DocumentId>
ProjectManager::computeBuiltinVisibleDocumentIds(
    const cursiv::Document& document) const {
  auto ids = collectCoreBuiltinDocumentIds(builtinStandard(document.uri));
  ids.insert(document.id);
  return ids;
}

std::unordered_set<cursiv::DocumentId>
ProjectManager::computeStandaloneVisibleDocumentIds(
    const cursiv::Document& document) const {
  const auto tagged = builtinStandard(document.uri);
  auto ids =
      collectCoreBuiltinDocumentIds(tagged.empty() ? "ECSS_SMP_2020" : tagged);
  ids.insert(document.id);
  return ids;
}

}  // namespace xsmp
