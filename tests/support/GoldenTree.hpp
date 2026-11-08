#pragma once

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cstdlib>
#include <cursiv/cli.hpp>
#include <cursiv/services/ServiceRegistry.hpp>
#include <cursiv/utils/UriUtils.hpp>
#include <cursiv/workspace/Documents.hpp>
#include <filesystem>
#include <fstream>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>
#include <xsmp/CliWorkspace.hpp>
#include <xsmp/DocumentGenerator.hpp>
#include <xsmp/tas-mdk/TasMdkContribution.hpp>

#include "support/Files.hpp"

// The one way to compare generated files: a whole tree against a golden tree,
// byte for byte and in both directions. With `XSMP_UPDATE_GOLDENS=1` the
// goldens are rewritten instead, so that a change can be reviewed in a diff.
//
// A case of `tests/fixtures/generator/<generator>/<case>/` has:
//   input/          The project, with its documents in `input/smdl`.
//   expected/       The output, when it is the same under both standards.
//   expected-2020/  Otherwise the output of each standard.
//   expected-2025/
//   stubs/          The external headers that a C++ golden needs to compile.
//
// Without `input/xsmp.project`, the case runs under both standards. With one,
// it runs under the only standard that this descriptor names.
namespace xsmp {

inline bool updatingGoldens() {
  const auto* value = std::getenv("XSMP_UPDATE_GOLDENS");
  return value && std::string_view(value) == "1";
}

// The bytes of each file, by its path from the root of the tree.
using FileTree = std::map<std::string, std::string>;

inline FileTree readTree(const std::filesystem::path& root) {
  FileTree tree;
  if (!std::filesystem::is_directory(root)) return tree;
  for (const auto& entry : std::filesystem::recursive_directory_iterator(root))
    if (entry.is_regular_file())
      tree.emplace(entry.path().lexically_relative(root).generic_string(),
                   readFile(entry.path()));
  return tree;
}

inline void writeTree(const FileTree& tree, const std::filesystem::path& root) {
  std::filesystem::remove_all(root);
  for (const auto& [relative, content] : tree) {
    const auto path = root / relative;
    std::filesystem::create_directories(path.parent_path());
    std::ofstream(path, std::ios::binary) << content;
  }
}

// `text` with its control characters and other bytes outside ASCII as escapes.
inline std::string printableLine(std::string_view text) {
  static constexpr std::string_view digits = "0123456789ABCDEF";
  std::string result;
  for (const auto c : text) {
    const auto byte = static_cast<unsigned char>(c);
    if (byte >= 0x20 && byte < 0x7F) {
      result += c;
    } else {
      result += "\\x";
      result += digits[byte >> 4];
      result += digits[byte & 0xF];
    }
  }
  return result;
}

inline std::vector<std::string_view> linesOf(std::string_view text) {
  std::vector<std::string_view> lines;
  while (!text.empty()) {
    const auto end = std::min(text.find('\n'), text.size());
    lines.push_back(text.substr(0, end));
    text.remove_prefix(std::min(end + 1, text.size()));
  }
  return lines;
}

// Where `actual` first departs from `golden`, for two different files.
inline std::string firstDifferingLine(std::string_view golden,
                                      std::string_view actual) {
  const auto expected = linesOf(golden), generated = linesOf(actual);
  const auto [left, right] = std::ranges::mismatch(expected, generated);
  std::ostringstream out;
  out << "line " << (left - expected.begin()) + 1 << '\n';
  out << "  golden: "
      << (left == expected.end() ? "<end of file>" : printableLine(*left))
      << '\n';
  out << "  actual: "
      << (right == generated.end() ? "<end of file>" : printableLine(*right))
      << '\n';
  // The same lines, and the same bytes: only the last line ending differs.
  if (left == expected.end() && right == generated.end())
    out << "  (the last line ending differs)\n";
  return out.str();
}

// A readable account of the differences, empty when the trees are the same.
inline std::string describeDifference(const FileTree& golden,
                                      const FileTree& actual) {
  std::ostringstream out;
  for (const auto& [relative, content] : golden)
    if (!actual.contains(relative))
      out << "Missing from the output: " << relative << '\n';
  for (const auto& [relative, content] : actual)
    if (!golden.contains(relative))
      out << "Not in the golden: " << relative << '\n';
  std::vector<std::string> differing;
  for (const auto& [relative, content] : golden)
    if (const auto found = actual.find(relative);
        found != actual.end() && found->second != content)
      differing.push_back(relative);
  if (!differing.empty()) {
    const auto& first = differing.front();
    out << first << ", "
        << firstDifferingLine(golden.at(first), actual.at(first));
    if (differing.size() > 1)
      out << "and " << differing.size() - 1 << " more differing files.\n";
  }
  return out.str();
}

inline std::string goldenFailure(const std::filesystem::path& golden,
                                 const std::string& difference) {
  return golden.generic_string() + " differs:\n" + difference
         + "If this change is intended, run with XSMP_UPDATE_GOLDENS=1 and "
           "review the diff.";
}

// Compares the files of folder `actual` with the golden tree of folder
// `golden`, or rewrites the golden with them.
inline void expectMatchesGolden(const std::filesystem::path& actual,
                                const std::filesystem::path& golden) {
  const auto generated = readTree(actual);
  if (updatingGoldens()) {
    writeTree(generated, golden);
    return;
  }
  const auto difference = describeDifference(readTree(golden), generated);
  EXPECT_TRUE(difference.empty()) << goldenFailure(golden, difference);
}

// Pins what a generated file would take from the machine and the moment.
class PinnedGenerationStamp {
 public:
  // By default 2026-01-31T13:45:09Z.
  explicit PinnedGenerationStamp(const char* epoch = "1769867109") {
    set("SOURCE_DATE_EPOCH", epoch);
    set("USER", "golden-user");
  }

  ~PinnedGenerationStamp() {
    for (const auto& [name, value] : _saved) {
      if (value)
        assign(name.c_str(), value->c_str());
      else
        clear(name.c_str());
    }
  }

  PinnedGenerationStamp(const PinnedGenerationStamp&) = delete;
  PinnedGenerationStamp& operator=(const PinnedGenerationStamp&) = delete;

 private:
  static void assign(const char* name, const char* value) {
#ifdef _WIN32
    _putenv_s(name, value);
#else
    setenv(name, value, 1);
#endif
  }

  static void clear(const char* name) {
#ifdef _WIN32
    _putenv_s(name, "");
#else
    unsetenv(name);
#endif
  }

  void set(const char* name, const char* value) {
    const auto* previous = std::getenv(name);
    _saved.emplace_back(
        name, previous ? std::optional<std::string>(previous) : std::nullopt);
    assign(name, value);
  }

  std::vector<std::pair<std::string, std::optional<std::string>>> _saved;
};

// The workspace of a case, under one standard, with its documents built.
struct GoldenWorkspace {
  std::unique_ptr<XsmpSharedCoreServices> shared = createXsmpSharedServices();
  const Project* project = nullptr;
  std::string standard;

  explicit GoldenWorkspace(const std::filesystem::path& root) {
    registerLanguages(*shared);
    registerTasMdk(*shared);
    cursiv::initialize_root(root.string(), *shared);
    const auto descriptor = shared->documents->getDocument(
        cursiv::path_to_file_uri((root / "xsmp.project").string()));
    if (descriptor)
      project = cursiv::ast_cast<const Project>(descriptor->parseResult.value);
  }

  const XsmpcatCoreServices& language() const {
    return dynamic_cast<const XsmpcatCoreServices&>(
        shared->serviceRegistry->getServices({}, "xsmpcat"));
  }

  // The documents of the project: each catalogue and deployment document.
  std::vector<const cursiv::AstNode*> nodes() const {
    std::vector<const cursiv::AstNode*> result;
    if (!project) return result;
    for (const auto* document : projectDocuments(*shared, *project, false)) {
      EXPECT_TRUE(document->parseSucceeded()) << document->uri;
      if (cursiv::ast_cast<const Document>(document->parseResult.value))
        result.push_back(document->parseResult.value);
    }
    return result;
  }
};

using GoldenGenerator =
    std::function<void(GoldenWorkspace&, const std::filesystem::path& output)>;

inline constexpr std::array<std::string_view, 2> goldenStandards{
    "ECSS_SMP_2020", "ECSS_SMP_2025"};

// `ECSS_SMP_2025` gives `expected-2025`.
inline std::string goldenFolderOf(std::string_view standard) {
  return "expected-" + std::string(standard.substr(standard.size() - 4));
}

// The standard named by `input/xsmp.project`, if the case has one.
inline std::optional<std::string> pinnedStandard(
    const std::filesystem::path& caseDirectory) {
  const auto descriptor = caseDirectory / "input/xsmp.project";
  if (!std::filesystem::exists(descriptor)) return std::nullopt;
  const auto text = readFile(descriptor);
  for (const auto& standard : goldenStandards)
    if (text.find(standard) != std::string::npos) return std::string(standard);
  ADD_FAILURE() << descriptor << " does not name its standard";
  return std::nullopt;
}

// What `generate` writes for `input` under `standard`.
inline FileTree generateGoldenTree(const std::filesystem::path& caseDirectory,
                                   std::string_view standard,
                                   const GoldenGenerator& generate) {
  const auto name = caseDirectory.filename().string();
  const ScratchDirectory scratch, output;
  const auto root = scratch.path / name;
  std::filesystem::copy(caseDirectory / "input", root,
                        std::filesystem::copy_options::recursive);
  if (!std::filesystem::exists(root / "xsmp.project"))
    std::ofstream(root / "xsmp.project", std::ios::binary)
        << "project '" << name << "' using '" << standard
        << "'\nsource 'smdl'\n";
  GoldenWorkspace workspace(root);
  workspace.standard = standard;
  if (!workspace.project) {
    ADD_FAILURE() << "No project in " << caseDirectory;
    return {};
  }
  generate(workspace, output.path);
  return readTree(output.path);
}

// Generates a case with `generate` under each of its standards, and compares
// the output with its goldens.
inline void expectGoldenCase(const std::filesystem::path& caseDirectory,
                             const GoldenGenerator& generate) {
  const PinnedGenerationStamp pinned;
  std::vector<std::string> standards;
  if (const auto standard = pinnedStandard(caseDirectory))
    standards.push_back(*standard);
  else
    standards.assign(goldenStandards.begin(), goldenStandards.end());

  std::vector<FileTree> trees;
  for (const auto& standard : standards)
    trees.push_back(generateGoldenTree(caseDirectory, standard, generate));
  const auto same = standards.size() > 1 && trees.front() == trees.back();
  const auto shared = caseDirectory / "expected";

  if (updatingGoldens()) {
    std::filesystem::remove_all(shared);
    for (const auto& standard : goldenStandards)
      std::filesystem::remove_all(caseDirectory / goldenFolderOf(standard));
    if (same) {
      writeTree(trees.front(), shared);
    } else {
      for (std::size_t i = 0; i < standards.size(); ++i)
        writeTree(trees[i], caseDirectory / goldenFolderOf(standards[i]));
    }
    return;
  }

  // Each tree is in its one place, so that a reader knows where to look.
  if (same) {
    for (const auto& standard : goldenStandards)
      EXPECT_FALSE(
          std::filesystem::exists(caseDirectory / goldenFolderOf(standard)))
          << "Both standards give the same output in " << caseDirectory;
    const auto difference = describeDifference(readTree(shared), trees.front());
    EXPECT_TRUE(difference.empty()) << goldenFailure(shared, difference);
    return;
  }
  EXPECT_FALSE(std::filesystem::exists(shared))
      << "The standards give different outputs in " << caseDirectory;
  for (std::size_t i = 0; i < standards.size(); ++i) {
    const auto golden = caseDirectory / goldenFolderOf(standards[i]);
    const auto difference = describeDifference(readTree(golden), trees[i]);
    EXPECT_TRUE(difference.empty()) << goldenFailure(golden, difference);
  }
}

}  // namespace xsmp
