#pragma once

#include <array>
#include <cursiv/services/DefaultSharedCoreService.hpp>
#include <cursiv/utils/SimpleCache.hpp>
#include <cursiv/workspace/AstNodeDescription.hpp>
#include <cursiv/workspace/DocumentBuilder.hpp>
#include <memory>
#include <span>
#include <string_view>
#include <unordered_set>
#include <variant>
#include <vector>
#include <xsmp/ast.hpp>

namespace cursiv {

struct SharedCoreServices;
struct Document;

}  // namespace cursiv

namespace xsmp {

/// SMP standard revisions recognised by `Project::standard` / `Dependency`
/// matching.
inline constexpr std::array<std::string_view, 2> kSmpStandards{"ECSS_SMP_2020",
                                                               "ECSS_SMP_2025"};

/// The `@<standard>` tag of a built-in catalogue's file name (as in
/// "xsmp:/ecss.smp%40ECSS_SMP_2020.xsmpcat"), or empty when untagged.
[[nodiscard]] std::string_view builtinStandard(std::string_view uri);

/// Resolves which `xsmp.project` (if any) owns a document, the transitive
/// `dependency` closure of a project, and the resulting set of documents
/// visible for scoping/validation from a given document (documents of the
/// dependency closure + per-standard builtin filtering, or a builtin/standalone
/// fallback when no project owns the document). Contribution payloads are
/// visible only to the project activating their owning profile/tool; core
/// builtins remain implicit and dependencies do not inherit contributions.
/// Lives on `xsmp::XsmpSharedCoreServices`, and every container of it must
/// construct it right after the cursiv defaults are installed:
/// `xsmp::createXsmpSharedServices()` does it for the core-only container; the
/// LSP server and tests do it the same way.
class ProjectManager final : public cursiv::DefaultSharedCoreService,
                             private cursiv::BuildPhaseListener,
                             private cursiv::DocumentUpdateListener {
 public:
  explicit ProjectManager(cursiv::SharedCoreServices& shared);
  ~ProjectManager() override;

  // Metadata changes need no virtual-file read: the caller rebuilds held
  // documents.
  void contributionsChanged();

  /// Every `Project` declared anywhere in the workspace.
  [[nodiscard]] std::vector<const Project*> getProjects() const;

  /// The first (in `Documents::all()` order) declared `Project` named `name`,
  /// or nullptr.
  [[nodiscard]] const Project* getProjectByName(std::string_view name) const;

  /// The `Project` that owns `document`: among the `xsmp.project` files whose
  /// directory contains it and whose `source` folders (resolved relative to
  /// that directory) contain it too, the one in the deepest directory. nullptr
  /// when no project owns it (a builtin or a standalone document).
  [[nodiscard]] const Project* getProject(
      const cursiv::Document& document) const;

  /// The transitive `dependency` closure of `project`, INCLUDING `project`
  /// itself.
  [[nodiscard]] std::unordered_set<const Project*> getDependencies(
      const Project& project) const;

  /// The set of document ids visible for scoping/validation from `document`:
  /// when `document` has an owning project, the descriptors of its dependency
  /// closure plus every document one of those projects owns plus builtins
  /// matching the project's `standard` (or untagged); otherwise a
  /// builtin- or standalone-visibility fallback (self plus same-standard/
  /// untagged builtins, defaulting to `ECSS_SMP_2020` for a standalone
  /// non-builtin document).
  [[nodiscard]] std::shared_ptr<const std::unordered_set<cursiv::DocumentId>>
  getVisibleDocumentIds(const cursiv::Document& document) const;

 private:
  const Project* doGetProject(const cursiv::Document& document) const;
  void collectDependencies(const Project& project,
                           std::unordered_set<const Project*>& out) const;

  std::unordered_set<cursiv::DocumentId> computeVisibleDocumentIds(
      const Project& project) const;

  // A built-in sees itself and the core built-ins of its own standard.
  std::unordered_set<cursiv::DocumentId> computeBuiltinVisibleDocumentIds(
      const cursiv::Document& document) const;

  // A document of no project sees itself and the core built-ins of its tagged
  // standard, `ECSS_SMP_2020` by default.
  std::unordered_set<cursiv::DocumentId> computeStandaloneVisibleDocumentIds(
      const cursiv::Document& document) const;

  // The built-ins that are not contribution payloads: the untagged ones and
  // those of `standard`. An empty `standard` gives the untagged ones only.
  std::unordered_set<cursiv::DocumentId> collectCoreBuiltinDocumentIds(
      std::string_view standard) const;

  void clearCaches();

  // A deleted document has left the store when this hears of it, and any kind
  // of document may be gone: every modeling document but the built-ins relinks
  // in the same update. An edit reaches the others through the index manager.
  void onUpdate(std::span<const std::string_view> changed,
                std::span<const std::string_view> deleted) override;

  void onBuildPhase(cursiv::DocumentState reached,
                    std::span<cursiv::Document* const> documents,
                    const cursiv::CancellationToken& token) override;

  // The caches below read the projects and which documents exist, so an edit of
  // anything else leaves them right. They are emptied when an update changes
  // those, and again once every document of that update is parsed: a project
  // not parsed yet would give a wrong answer.
  cursiv::SimpleCache<std::monostate, std::vector<const Project*>> _projects;
  cursiv::SimpleCache<cursiv::DocumentId, const Project*> _projectCache;

  // By project, or by document for one no project owns.
  cursiv::SimpleCache<
      const void*,
      std::shared_ptr<const std::unordered_set<cursiv::DocumentId>>>
      _visibleCache;

  std::size_t _documentCount = 0;
  bool _clearWhenParsed = false;

  cursiv::DocumentBuilder* _builder = nullptr;
};

}  // namespace xsmp
