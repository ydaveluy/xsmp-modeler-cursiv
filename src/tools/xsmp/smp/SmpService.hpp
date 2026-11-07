#pragma once

#include <cursiv/services/SharedCoreServices.hpp>
#include <cursiv/utils/Cancellation.hpp>
#include <cursiv/workspace/Document.hpp>
#include <filesystem>
#include <set>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

// Conversion between SMP XML documents and XSMP text, and the workspace
// services that show an XML file as a read-only XSMP document: its mirror.

namespace xsmp {

// An SMP XML document rendered as XSMP text.
struct ImportResult {
  // "catalogue", "configuration", "linkbase", "assembly" or "schedule".
  std::string kind;
  std::string content;
  // The input path with its XSMP extension, unless the caller gave another.
  std::filesystem::path outputPath;
  std::vector<std::string> warnings;
};

struct ImportOptions {
  // Explicit workspace roots. Standalone imports search the input directory.
  std::vector<std::filesystem::path> searchRoots;
  // The workspace, whose built-in catalogues need no second parse.
  const cursiv::Documents* documents = nullptr;
  // Where to note the file name of each link, found or not: a file of that
  // name can change the render.
  std::set<std::string>* references = nullptr;
};

// Rendering performs no writes. Import validates the generated DSL syntax
// before creating its output and refuses existing files unless overwrite=true.
ImportResult renderImportedDocument(const std::filesystem::path& inputPath,
                                    const ImportOptions& options = {});
ImportResult importFile(const std::filesystem::path& inputPath,
                        std::filesystem::path outputPath = {},
                        bool overwrite = false,
                        const ImportOptions& options = {});

// The SMP XML of `document`. `standard` is "ECSS_SMP_2020" or "ECSS_SMP_2025";
// empty means the standard of its project, or else 2020. Throws on another
// standard, and on a value that cannot be written.
std::string exportDocument(const cursiv::Document& document,
                           std::string_view standard = {});

// Writes that XML to `directory/smdl-gen`, with a `.smppkg` descriptor for a
// catalogue. Returns the path of the XML file.
std::filesystem::path generateDocument(const cursiv::Document& document,
                                       const std::filesystem::path& directory,
                                       std::string_view standard = {});

// A directory that the workspace scan and the search for external documents
// skip: hidden and dependency directories, build outputs and `smdl-gen`, the
// folder that SMP export writes to.
[[nodiscard]] bool ignoredDirectory(std::string_view name);

// Mirror URIs, each in one of the lists.
struct MirrorRefreshResult {
  std::vector<std::string> changed;
  std::vector<std::string> deleted;
};

// Adds automatic mirror loading at workspace startup and a filesystem adapter
// that re-reads a mirror when its source XML changes. Project and XML updates
// refresh the mirrors through the installed document builder.
void installSmpWorkspaceManager(cursiv::SharedCoreServices& shared);

// The mirrors added, changed or removed since the last call, including the ones
// of the initial workspace loading.
MirrorRefreshResult takeMirrorChanges(cursiv::SharedCoreServices& shared);

// Invalid XML has no native mirror document, but still blocks project actions.
// Pairs contain the physical source URI and its import diagnostic.
std::vector<std::pair<std::string, std::string>> sourceErrors(
    const cursiv::SharedCoreServices& shared);

// Import warnings are published on the physical XML source without blocking
// project actions. Several warnings can refer to the same source URI.
std::vector<std::pair<std::string, std::string>> sourceWarnings(
    const cursiv::SharedCoreServices& shared);

}  // namespace xsmp
