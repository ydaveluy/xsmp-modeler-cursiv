#include <gtest/gtest.h>

#include <algorithm>
#include <cctype>
#include <cursiv/utils/UriUtils.hpp>
#include <cursiv/workspace/DocumentBuilder.hpp>
#include <filesystem>
#include <fstream>
#include <map>
#include <string>
#include <string_view>
#include <vector>
#include <xsmp/Contributions.hpp>
#include <xsmp/smp/SmpService.hpp>

#include "support/Files.hpp"
#include "support/GoldenTree.hpp"

// A case of `tests/fixtures/generator/smp-import/<case>/` holds one of two
// forms:
//   input/*.smp*        Each XML document is imported to the same place in
//                       `expected`.
//   input/smdl/*.xsmp*  Both steps: each native document is exported to
//                       `smdl-gen`, and each XML is imported to `imported`.
//                       Exporting what was imported must give the same XML.
//
// A case also checks what a tree cannot show: the warnings of each import, and
// that the imported documents parse and validate.

namespace {

using std::filesystem::path;

const path& importCases() {
  static const auto folder =
      path(XSMP_FIXTURES_DIR) / "generator" / "smp-import";
  return folder;
}

// What each import warns of, by its path in `smp-import`. Any other import
// warns of nothing. `__ROOT__` stands for the input folder of the case.
const std::map<std::string, std::vector<std::string>>& expectedWarnings() {
  static const std::map<std::string, std::vector<std::string>> warnings{
      {"assemblies/invocations.smpasb",
       {"Unsupported assembly invocation type 'Assembly:CustomInvocation'.",
        "Missing model implementation; using placeholder string "
        "implementation.",
        "Could not resolve assembly reference 'missing/dep.smpasb'; using "
        "'dep'.",
        "Could not resolve assembly configuration 'configs/cfg.smpcfg'; using "
        "'cfg'.",
        "Could not resolve assembly link base 'links/demo.smplnk'; using "
        "'demo'."}},
      {"assemblies/unknown.smpasb",
       {"Unsupported assembly invocation type 'Assembly:CustomInvocation'.",
        "Missing model implementation; using placeholder string "
        "implementation."}},
      {"catalogue-members/parameters.smpcat",
       {"Parameter default 'new int(3)' is not representable in XSMP and was "
        "discarded."}},
      {"catalogue-members/realization.smpcat",
       {"Realization 'logger' of 'M' is not representable in XSMP and was "
        "discarded."}},
      {"catalogue-values/numbers.smpcat",
       {"The value '-INF' is not representable in XSMP and was replaced by "
        "0."}},
      {"catalogue-values/start_index.smpcat",
       {"Catalogue array default value StartIndex='1' is not representable in "
        "XSMP and was discarded."}},
      {"configuration-values/numbers.smpcfg",
       {"The value 'INF' is not representable in XSMP and was replaced by 0.",
        "The value 'NaN' is not representable in XSMP and was replaced by 0."}},
      {"dependency-ambiguous/main.smpcat",
       {"Multiple SMP files match external field type href "
        "'dependency.smpcat': '__ROOT__/a/dependency.smpcat', "
        "'__ROOT__/b/dependency.smpcat'. Not selecting one."}},
      // Several files match, in the order of their paths. `smdl-gen` is
      // ignored.
      {"dependency-walk/inbox/main.smpcat",
       {"Multiple SMP files match external field type href 'd.smpcat': "
        "'__ROOT__/deps/one/d.smpcat', '__ROOT__/deps/two/d.smpcat'. Not "
        "selecting one."}},
      {"links/demo.smplnk",
       {"Unsupported link type 'LinkBase:CustomLink'; importing as field "
        "link."}},
      {"missing-names/missing.smpasb",
       {"Missing model implementation; using placeholder string "
        "implementation."}},
      {"schedule-task-title/main.smpsed",
       {"Could not resolve external task reference id 'Hidden/MissingTask' "
        "from 'external.smpsed#Hidden/MissingTask'; falling back to title or "
        "fragment text."}},
      {"schedules/boot_flow.smpsed",
       {"Unsupported schedule activity type 'Schedule:CustomActivity'.",
        "Unsupported schedule event type 'Schedule:CustomEvent'."}},
      {"schedules/unknown.smpsed",
       {"Unsupported schedule activity type 'Schedule:CustomActivity'.",
        "Unsupported schedule event type 'Schedule:CustomEvent'."}},
  };
  return warnings;
}

// The imported documents that do not validate, by their path in `smp-import`,
// and why.
const std::map<std::string, std::string>& documentsWithErrors() {
  constexpr auto nothingBehindTheNames =
      "It names documents that this case leaves out, to show the warnings.";
  constexpr auto ambiguous =
      "Two documents have the same name, to show that none is selected.";
  constexpr auto placeholder =
      "The XML leaves out a name, which a placeholder stands for.";
  constexpr auto referenceDocument =
      "A reference document, written without the catalogue it refers to.";
  static const std::map<std::string, std::string> documents{
      {"assemblies/invocations.xsmpasb", nothingBehindTheNames},
      {"catalogue-members/no_value.xsmpcat",
       "The XML has a constant without its value, and nothing is made up."},
      {"dependency-ambiguous/a/dependency.xsmpcat", ambiguous},
      {"dependency-ambiguous/b/dependency.xsmpcat", ambiguous},
      {"dependency-ambiguous/main.xsmpcat", ambiguous},
      {"dependency-walk/deps/one/d.xsmpcat", ambiguous},
      {"dependency-walk/deps/two/d.xsmpcat", ambiguous},
      {"dependency-walk/deps/two/e.xsmpcat", ambiguous},
      {"dependency-walk/inbox/main.xsmpcat", ambiguous},
      {"dependency-walk/smdl-gen/f.xsmpcat", ambiguous},
      {"missing-names/missing.xsmpasb", placeholder},
      {"missing-names/missing.xsmpcat", placeholder},
      {"missing-names/missing.xsmpcfg", placeholder},
      {"missing-names/missing.xsmplnk", placeholder},
      {"missing-names/missing.xsmpsed", placeholder},
      {"reference-documents/test.xsmpasb", referenceDocument},
      {"reference-documents/test.xsmpcat",
       "This catalogue holds invalid constructs on purpose."},
      {"reference-documents/test.xsmpcfg", referenceDocument},
      {"reference-documents/test.xsmplnk", referenceDocument},
      {"reference-documents/test.xsmpsed", referenceDocument},
      {"schedule-task-title/main.xsmpsed", nothingBehindTheNames},
  };
  return documents;
}

std::vector<std::string> goldenImportCases() {
  std::vector<std::string> cases;
  for (const auto& entry : std::filesystem::directory_iterator(importCases()))
    if (entry.is_directory()) cases.push_back(entry.path().filename().string());
  std::ranges::sort(cases);
  return cases;
}

bool isImportableXml(const path& file) {
  const auto extension = file.extension().string();
  return extension.starts_with(".smp") && extension != ".smppkg";
}

// Imports each XML document under `from` to the same place under `to`.
void importTree(const path& from, const path& to, std::string_view label,
                const path& root) {
  std::vector<path> documents;
  for (const auto& entry : std::filesystem::recursive_directory_iterator(from))
    if (entry.is_regular_file() && isImportableXml(entry.path()))
      documents.push_back(entry.path());
  std::ranges::sort(documents);
  for (const auto& document : documents) {
    const auto relative = document.lexically_relative(from);
    const auto name = std::string(label) + "/" + relative.generic_string();
    const auto imported =
        xsmp::renderImportedDocument(document, {.searchRoots = {from}});
    auto output = to / relative;
    output.replace_extension(imported.outputPath.extension());
    std::filesystem::create_directories(output.parent_path());
    std::ofstream(output, std::ios::binary) << imported.content;

    std::vector<std::string> expected;
    if (const auto found = expectedWarnings().find(name);
        found != expectedWarnings().end())
      expected = found->second;
    for (auto& warning : expected)
      for (auto at = warning.find("__ROOT__"); at != warning.npos;
           at = warning.find("__ROOT__", at + root.string().size()))
        warning.replace(at, 8, root.string());
    EXPECT_EQ(imported.warnings, expected) << name;
  }
}

// The errors that validation gives for `document`, one per line.
std::string errorsOf(const cursiv::Document& document) {
  std::string errors;
  for (const auto& diagnostic : document.diagnostics)
    if (diagnostic.severity == cursiv::DiagnosticSeverity::Error)
      errors += diagnostic.message + "\n";
  return errors;
}

// The project of an imported tree, with its documents validated.
struct ImportedProject {
  xsmp::ScratchDirectory scratch;
  path root = scratch.path / "imported";
  std::vector<cursiv::Document*> documents;
  std::unique_ptr<xsmp::GoldenWorkspace> workspace;

  ImportedProject(const path& tree, std::string_view standard) {
    std::filesystem::create_directories(root);
    std::filesystem::copy(tree, root / "smdl",
                          std::filesystem::copy_options::recursive);
    std::ofstream(root / "xsmp.project", std::ios::binary)
        << "project 'imported' using '" << standard << "'\nsource 'smdl'\n";
    workspace = std::make_unique<xsmp::GoldenWorkspace>(root);
    auto& shared = *workspace->shared;
    documents = xsmp::projectDocuments(shared, *workspace->project, true);
    for (auto* document : documents)
      if (document->state >= cursiv::DocumentState::IndexedReferences)
        shared.documentBuilder->resetToState(
            *document, cursiv::DocumentState::IndexedReferences);
    shared.documentBuilder->build(documents, {.validation = true});
  }

  // The path of `document` in the imported tree.
  std::string relative(const cursiv::Document& document) const {
    const auto file = cursiv::file_uri_to_path(document.uri);
    return file ? path(*file).lexically_relative(root / "smdl").generic_string()
                : document.uri;
  }
};

// Each imported document parses, and validates unless it is listed.
void expectValidDocuments(const ImportedProject& project,
                          std::string_view label, std::string_view folder) {
  for (const auto* document : project.documents) {
    if (!cursiv::ast_cast<const xsmp::Document>(document->parseResult.value))
      continue;
    const auto name = std::string(label) + "/" + std::string(folder)
                      + project.relative(*document);
    EXPECT_TRUE(document->parseSucceeded()) << name;
    const auto errors = errorsOf(*document);
    if (documentsWithErrors().contains(name))
      EXPECT_FALSE(errors.empty()) << name
                                   << " validates: remove it from the "
                                      "list";
    else
      EXPECT_EQ(errors, "") << name;
  }
}

// The standard that the XML of a case is written for. A custom namespace says
// nothing, and gets the latest.
std::string standardOfXml(const path& input) {
  for (const auto& entry : std::filesystem::recursive_directory_iterator(input))
    if (entry.is_regular_file()
        && xsmp::readFile(entry.path()).find("smp/2019/Smdl/")
               != std::string::npos)
      return "ECSS_SMP_2020";
  return "ECSS_SMP_2025";
}

// Text to XML, and XML back to text, with the same XML from what was imported.
void expectRoundTrip(const std::string& name) {
  xsmp::expectGoldenCase(
      importCases() / name,
      [&](xsmp::GoldenWorkspace& workspace, const path& output) {
        const std::vector<std::string> smp{"smp"};
        xsmp::registry(*workspace.shared)
            .generateBatch(workspace.nodes(), output, smp);
        importTree(output / "smdl-gen", output / "imported", name + "/smdl-gen",
                   output / "smdl-gen");
        const ImportedProject back(output / "imported", workspace.standard);
        expectValidDocuments(back, name, "imported/");
        for (const auto* document : back.documents) {
          if (!cursiv::ast_cast<const xsmp::Document>(
                  document->parseResult.value))
            continue;
          auto xml = output / "smdl-gen" / back.relative(*document);
          xml.replace_extension(".smp" + xml.extension().string().substr(5));
          const auto difference = xsmp::describeDifference(
              {{xml.filename().string(), xsmp::readFile(xml)}},
              {{xml.filename().string(), xsmp::exportDocument(*document)}});
          EXPECT_EQ(difference, "")
              << "Exporting what was imported, " << workspace.standard;
        }
      });
}

class SmpImportGolden : public ::testing::TestWithParam<std::string> {};

}  // namespace

TEST_P(SmpImportGolden, MatchesItsTree) {
  const auto& name = GetParam();
  const auto input = importCases() / name / "input";
  if (std::filesystem::exists(input / "smdl")) {
    expectRoundTrip(name);
    return;
  }
  const xsmp::ScratchDirectory output;
  importTree(input, output.path, name, input);
  xsmp::expectMatchesGolden(output.path, importCases() / name / "expected");
  const ImportedProject project(output.path, standardOfXml(input));
  expectValidDocuments(project, name, "");
}

INSTANTIATE_TEST_SUITE_P(
    Cases, SmpImportGolden, ::testing::ValuesIn(goldenImportCases()),
    [](const ::testing::TestParamInfo<std::string>& info) {
      auto name = info.param;
      std::ranges::replace_if(
          name,
          [](char c) { return !std::isalnum(static_cast<unsigned char>(c)); },
          '_');
      return name;
    });

// Rendering says what the document is, and writes nothing.
TEST(SmpImport, RenderingNamesTheKindAndWritesNothing) {
  for (const auto& [extension, kind] :
       {std::pair{".smpcat", "catalogue"},
        std::pair{".smpcfg", "configuration"}, std::pair{".smpasb", "assembly"},
        std::pair{".smplnk", "linkbase"}, std::pair{".smpsed", "schedule"}}) {
    SCOPED_TRACE(extension);
    const xsmp::ScratchDirectory dir;
    const auto input = dir.path / (std::string("test") + extension);
    std::filesystem::copy_file(
        importCases() / "reference-documents/input" / input.filename(), input);
    const auto imported = xsmp::renderImportedDocument(input);
    EXPECT_EQ(imported.kind, kind);
    EXPECT_EQ(imported.outputPath.extension(),
              std::string(".x") + (extension + 1));
    EXPECT_EQ(xsmp::readTree(dir.path).size(), 1);
  }
}

TEST(SmpImport, AnAssemblyWithoutARootModelIsRefused) {
  const xsmp::ScratchDirectory dir;
  EXPECT_THROW(
      xsmp::renderImportedDocument(dir.write(
          "broken.smpasb",
          R"(<Assembly xmlns="urn:custom/Assembly" Name="Broken"><ComponentConfiguration InstancePath="."/></Assembly>)")),
      std::runtime_error);
}

TEST(SmpImport, DocumentDetectionRequiresMatchingRootNamespace) {
  xsmp::ScratchDirectory dir;
  for (const auto* xml :
       {R"(<Catalogue Name="Foreign"/>)",
        R"(<Catalogue xmlns="urn:foreign" Name="Foreign"/>)",
        R"(<c:Catalogue xmlns:c="urn:any/Configuration" Name="Foreign"/>)"})
    EXPECT_THROW(xsmp::renderImportedDocument(dir.write("foreign.smpcat", xml)),
                 std::runtime_error);
  EXPECT_EQ(
      xsmp::renderImportedDocument(
          dir.write(
              "valid.smpcat",
              R"(<c:Catalogue xmlns:c="urn:custom/Catalogue" Name="Valid"/>)"))
          .kind,
      "catalogue");
}

TEST(SmpImport, XmlBooleansRejectInvalidAndEmptyLexicalValues) {
  xsmp::ScratchDirectory dir;
  for (const auto* value : {"invalid", "TRUE", "", "2"}) {
    SCOPED_TRACE(value);
    const auto prefix = std::string(
        R"(<Catalogue xmlns="urn:custom/Catalogue" Name="Demo" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"><Namespace Name="n"><Type xsi:type="Catalogue:Model" Name="M" Abstract=")");
    EXPECT_THROW(
        xsmp::renderImportedDocument(dir.write(
            "bool.smpcat", prefix + value + R"("/></Namespace></Catalogue>)")),
        std::runtime_error);
    EXPECT_THROW(
        xsmp::renderImportedDocument(dir.write(
            "bool.smpcfg",
            std::string(
                R"(<Configuration xmlns="urn:custom/Configuration" Name="Demo" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"><Component Path="/Root"><FieldValue xsi:type="Types:BoolValue" Field="flag" Value=")")
                + value + R"("/></Component></Configuration>)")),
        std::runtime_error);
    EXPECT_THROW(
        xsmp::renderImportedDocument(dir.write(
            "bool.smpsed",
            std::string(
                R"(<Schedule xmlns="urn:custom/Schedule" Name="Demo" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"><Task Name="Main"><Activity xsi:type="Schedule:EmitGlobalEvent" synchronous=")")
                + value + R"("/></Task></Schedule>)")),
        std::runtime_error);
  }
}

TEST(SmpImport, RefusesOverwriteMalformedXmlAndUnsupportedRoots) {
  xsmp::ScratchDirectory dir;
  auto input = dir.write(
      "demo.smpcfg",
      R"(<Configuration xmlns="http://www.ecss.nl/smp/2025/Smdl/Configuration" Name="Demo"/>)");
  auto output = dir.write("demo.xsmpcfg", "existing source\n");
  EXPECT_THROW(xsmp::importFile(input, output), std::runtime_error);
  std::ifstream existing(output);
  std::string content{std::istreambuf_iterator<char>(existing), {}};
  EXPECT_EQ(content, "existing source\n");
  auto result = xsmp::importFile(input, output, true);
  EXPECT_EQ(result.outputPath, output);
  EXPECT_THROW(
      xsmp::renderImportedDocument(dir.write(
          "bad.smpcfg",
          "<Configuration "
          "xmlns=\"http://www.ecss.nl/smp/2025/Smdl/Configuration\">")),
      std::runtime_error);
  EXPECT_THROW(xsmp::renderImportedDocument(dir.write(
                   "package.smppkg", "<Package Name=\"Unsupported\"/>")),
               std::runtime_error);
  EXPECT_THROW(
      xsmp::renderImportedDocument(dir.write(
          "roots.smpcfg",
          R"(<Configuration xmlns="urn:custom/Configuration" Name="One"/><Configuration xmlns="urn:custom/Configuration" Name="Two"/>)")),
      std::runtime_error);
}
