#pragma once

#include <lsp/json/json.h>

#include <cursiv/services/SharedCoreServices.hpp>
#include <cursiv/syntax-tree/AstNode.hpp>
#include <cursiv/validation/ValidationCategory.hpp>
#include <filesystem>
#include <functional>
#include <map>
#include <span>
#include <string>
#include <vector>

// The profiles and tools a project can select in its `xsmp.project`. Each is a
// `Contribution`: its names and the callbacks that generate, scaffold and
// validate for it. A `Registry` holds those of one workspace: the built-in ones,
// and those of the extensions, which are libraries: see `Extension.hpp`.

namespace xsmp {

struct XsmpcatCoreServices;
struct XsmpSharedCoreServices;

// Whether an `apiVersion` of a manifest includes the API of this host.
bool supportsExtensionApiVersion(std::string_view range);

using Json = ::lsp::json::Value;

// What a scaffolder gets when a new project is created. The answers of the
// wizard are in `promptValues`, by `<contribution id>.<prompt id>`.
struct ScaffoldContext {
  std::string projectName;
  std::filesystem::path projectDir;

  // The contribution whose scaffolder is running.
  std::string contributionId;
  std::string selectedProfileId;
  std::vector<std::string> selectedToolIds;
  Json promptValues{::lsp::json::Object{}};

  // The projects the new project must depend on, for its `xsmp.project`.
  std::vector<std::string> dependencies;
  void writeFile(const std::filesystem::path&, std::string_view) const;
  void addDependency(std::string);

  // The answer to prompt `id` of this contribution, null when there is none.
  Json getPromptValue(std::string_view id) const;
};

using Generator =
    std::function<void(const cursiv::AstNode&, const std::filesystem::path&)>;

// A folder under the output directory where generation writes beside the files
// of the user, such as `python`. Generation lists the files it wrote there in
// `.xsmp/generated-files.json`, under the names of the generator and of the
// output, which the other XSMP tools use too. Once it has succeeded, it removes
// those it no longer writes.
struct TrackedFolder {
  std::string generator;
  std::string output;
  std::string folder;
};

// What one generation writes into one output directory: the trees of `nodes`,
// by the contributions `identifiers`.
struct GenerationGroup {
  std::filesystem::path directory;
  std::vector<std::string> identifiers;
  std::vector<const cursiv::AstNode*> nodes;
};

// One profile or tool: its names and the callbacks that do its work. An empty
// callback means the contribution has nothing to do at that step.
struct Contribution {
  std::string id;
  std::string label;

  // Other names `Registry::resolve` accepts for it.
  std::vector<std::string> aliases;
  std::vector<Generator> generators;

  // "profile" or "tool", a tool when empty.
  std::string kind;
  std::string description;

  // Who provides it. `Registry::add` fills this in for a built-in.
  std::string extensionId;
  bool defaultSelected = false;

  // Other names that still work, but are reported as deprecated.
  std::vector<std::string> deprecatedAliases;

  // The questions of the project wizard, as JSON objects: `id`, `label`, `type`
  // (`string`, `boolean` or `choice`), and maybe `defaultValue` and `choices`.
  std::vector<Json> wizardPrompts;

  // The folders under the output directory that generation alone writes to,
  // such as `src-gen`. Before it runs, links in them are removed. After it has
  // succeeded, so is any file that no generator gave to `writeFileIfChanged`.
  std::vector<std::string> generatedFolders;
  std::vector<TrackedFolder> trackedFolders;
  std::function<void(ScaffoldContext&)> scaffolder;

  // Register the checks of the contribution under the category they are given,
  // which a document runs when its project selects the contribution. Registration
  // runs under the workspace write lock, before activation.
  std::vector<std::function<void(XsmpSharedCoreServices&,
                                 const cursiv::ValidationCategory&)>>
      validations;
};

// All the profiles and tools of one workspace, by id. Lookups also accept an
// alias, and listings keep the order of registration.
class Registry {
 public:
  // Fills in an empty kind, label and extension id. Throws when the id is
  // empty, when an id or alias is already taken, or when a generated or tracked
  // folder is not inside the output directory or is `.xsmp`.
  void add(Contribution contribution);

  // Like `resolve`, but throws for an unknown name.
  const Contribution& get(std::string_view id) const;

  // Finds by id or alias. Null for an unknown or empty name.
  const Contribution* resolve(std::string_view id) const noexcept;

  // The contributions of `kind`, or all of them when empty, for a wizard to
  // list.
  Json summaries(std::string_view kind = {}) const;

  // The first of `kind` marked `defaultSelected`, else the first of `kind`.
  // Empty when there is none.
  std::string preferredId(std::string_view kind) const;

  // The prompts of the selected profile, then of the tools. Each one gains a
  // `contributionId` and a `key`. A name unknown or of another kind is skipped.
  Json wizardPrompts(std::string_view profile,
                     std::span<const std::string> tools) const;

  // Runs the scaffolders of the selected profile, then of the tools. Returns
  // `dependencies` and `failures`: one that throws does not stop the others.
  Json scaffoldProject(ScaffoldContext context) const;

  // Generates each group, then, once all have succeeded, removes what the
  // generation no longer writes: from the generated folders, and from the
  // tracked folders as their list tells. The groups share one record, as they
  // can share an output folder. Throws for an unknown name, for a tracked
  // folder in a generated one, and for what cannot be written or removed.
  void generate(std::span<const GenerationGroup>) const;

  // Adds the contributions that manifest entries describe. Returns `registered`
  // and `failures`: a bad entry is reported and does not stop the others, and
  // an entry its extension already registered is listed without being redone.
  // Rebuilds the whole workspace when something was added.
  Json registerManifestEntries(XsmpSharedCoreServices&, const Json& entries);

  // The documents held at the URIs of the documents of these entries, before
  // their extension registers anything: an editor reopened before the
  // extension came back holds them. Deleted before the registration, they give
  // way to the text of the extension.
  std::vector<std::string> placeholders(const XsmpSharedCoreServices&,
                                        const Json& entries) const;

  // The same for the `contributes.xsmp` entries of `package.json` in
  // `packageRoot`, using the `handler` of each. Throws when that file cannot be
  // read.
  Json registerPackage(XsmpSharedCoreServices&,
                       const std::filesystem::path& packageRoot);

  // Runs the generators of each selected contribution once, on this thread. An
  // unknown name or a null tree throws.
  void generateBatch(std::span<const cursiv::AstNode* const>,
                     const std::filesystem::path&,
                     std::span<const std::string> identifiers) const;

  const std::map<std::string, Contribution, std::less<>>& all() const {
    return entries;
  }

 private:
  std::map<std::string, Contribution, std::less<>> entries;
  std::vector<std::string> order;
};

// Write the starter files of a new project. A file that exists is kept.
void scaffoldXsmpSdkProject(const std::filesystem::path&,
                            std::string_view name);
void scaffoldPythonProject(const std::filesystem::path&, std::string_view name);
void scaffoldAdocProject(const std::filesystem::path&, std::string_view name);

// The generators keep a reference to `services`.
Registry makeBuiltinRegistry(const XsmpcatCoreServices& services);

// The registry of a workspace, made from the built-ins at first use.
Registry& registry(XsmpSharedCoreServices& shared);
Registry& registry(const cursiv::SharedCoreServices& shared);

// Adds a descriptor document per built-in, which is what a `profile` or `tool`
// reference in a project resolves to.
void loadBuiltinDescriptors(cursiv::SharedCoreServices& shared);

}  // namespace xsmp
