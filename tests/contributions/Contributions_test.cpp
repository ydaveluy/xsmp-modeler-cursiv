#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cursiv/utils/UriUtils.hpp>
#include <cursiv/workspace/DocumentBuilder.hpp>
#include <cursiv/workspace/Documents.hpp>
#include <filesystem>
#include <future>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <xsmp/CliWorkspace.hpp>
#include <xsmp/Contributions.hpp>
#include <xsmp/DocumentGenerator.hpp>
#include <xsmp/Json.hpp>
#include <xsmp/ast.hpp>
#include <xsmp/utils/FileWriter.hpp>

#include "support/Files.hpp"
#include "support/GeneratorTestSupport.hpp"

TEST(Contributions, AliasAndOtherKindAreNotIdempotentCanonicalRegistrations) {
  using namespace xsmp;
  auto shared = xsmp::createXsmpSharedServices();
  xsmp::registerLanguages(*shared);
  xsmp::ScratchDirectory directory;
  for (const auto& id : {std::string("canonical"), std::string("alias")}) {
    Registry registry;
    Contribution existing;
    existing.id = "canonical";
    existing.kind = "tool";
    existing.extensionId = "registry.extension";
    existing.aliases = {"alias"};
    registry.add(std::move(existing));
    xsmp::writeFileIfChanged(directory.path / "candidate.xsmpprofile",
                             "profile '" + id + "'\n");
    const auto result = registry.registerManifestEntries(
        *shared,
        JsonArray{JsonObject{{"extensionId", "registry.extension"},
                             {"extensionRoot", directory.path.string()},
                             {"descriptorPath", "candidate.xsmpprofile"},
                             {"handlerPath", "not-needed"},
                             {"apiVersion", "^2.0.0"}}});
    EXPECT_TRUE(result.object().get("registered").array().empty());
    ASSERT_EQ(result.object().get("failures").array().size(), 1);
    EXPECT_EQ(stringAt(result.object().get("failures").array().front().object(),
                       "phase"),
              "descriptor");
    EXPECT_EQ(registry.get("alias").kind, "tool");
  }
}

TEST(Contributions, OldNamesOfTheBuiltinsResolveAndWarn) {
  for (const auto* line : {"profile 'org.eclipse.xsmp.profile.xsmp-sdk'",
                           "profile 'org.eclipse.xsmp.profile.esa-cdk'",
                           "tool 'org.eclipse.xsmp.tool.python'",
                           "tool 'org.eclipse.xsmp.tool.adoc'",
                           "tool 'org.eclipse.xsmp.tool.smp'"}) {
    auto shared = xsmp::createXsmpSharedServices();
    xsmp::registerLanguages(*shared);
    auto document = shared->documents->createDocument(
        "file:///registry/xsmp.project",
        std::string("project 'Old'\n") + line + "\n");
    cursiv::Document* selected[]{document.get()};
    shared->documentBuilder->build(selected, {.validation = true});
    auto warned = false;
    for (const auto& diagnostic : document->diagnostics) {
      EXPECT_NE(diagnostic.severity, cursiv::DiagnosticSeverity::Error)
          << line << ": " << diagnostic.message;
      warned = warned || diagnostic.message.starts_with("Deprecated");
    }
    EXPECT_TRUE(warned) << line;
  }
}

TEST(Contributions, NonFileProjectDoesNotDereferenceAnEmptyOutputPath) {
  auto shared = xsmp::createXsmpSharedServices();
  xsmp::registerLanguages(*shared);
  auto document = shared->documents->createDocument(
      "untitled:///registry/xsmp.project", "project 'Virtual'\n");
  cursiv::Document* selected[]{document.get()};
  shared->documentBuilder->build(selected, {.validation = false});
  const auto* project =
      cursiv::ast_cast<xsmp::Project>(document->parseResult.value);
  ASSERT_NE(project, nullptr);
  const auto report = xsmp::generateProject(*shared, *project);
  EXPECT_TRUE(report.object().get("generatedProjects").array().empty());
  ASSERT_EQ(report.object().get("failures").array().size(), 1);
  EXPECT_NE(
      xsmp::stringAt(report.object().get("failures").array().front().object(),
                     "message")
          .find("local file URI"),
      std::string::npos);
}

TEST(Contributions, AGenerationWithAnUnknownNameWritesNothing) {
  xsmp::ScratchDirectory directory;
  xsmp::Registry registry;
  xsmp::Contribution tool;
  tool.id = "valid";
  tool.generatedFolders = {"a-gen"};
  tool.generators = {[&](const cursiv::AstNode&, const std::filesystem::path&) {
    xsmp::writeFileIfChanged(directory.path / "a-gen" / "A.txt", "written");
  }};
  registry.add(std::move(tool));
  const xsmp::Catalogue catalogue;
  const xsmp::GenerationGroup group{.directory = directory.path,
                                    .identifiers = {"valid", "unknown"},
                                    .nodes = {&catalogue}};
  EXPECT_THROW(registry.generate(std::span(&group, 1)), std::invalid_argument);
  EXPECT_FALSE(std::filesystem::exists(directory.path / "a-gen"));
}

TEST(Contributions, AGeneratedOrTrackedFolderStaysInsideTheOutput) {
  for (const auto* folder : {"", ".", "./", "..", "../x", "a/../..", "a/..",
                             "/etc", "/", ".xsmp", ".XSMP/list"}) {
    for (const auto tracked : {false, true}) {
      xsmp::Registry registry;
      xsmp::Contribution tool;
      tool.id = "tool";
      if (tracked)
        tool.trackedFolders = {
            {.generator = "g", .output = "o", .folder = folder}};
      else
        tool.generatedFolders = {folder};
      EXPECT_THROW(registry.add(std::move(tool)), std::invalid_argument)
          << folder;
    }
  }
  xsmp::Registry registry;
  xsmp::Contribution tool;
  tool.id = "tool";
  tool.trackedFolders = {{.generator = "", .output = "o", .folder = "f"}};
  EXPECT_THROW(registry.add(std::move(tool)), std::invalid_argument);
}

namespace {

// A project on disk with one tool that this test defines, whose generator is
// `generate`.
struct GeneratingProject {
  xsmp::ScratchDirectory directory;
  std::unique_ptr<xsmp::XsmpSharedCoreServices> shared =
      xsmp::createXsmpSharedServices();
  const xsmp::Project* project = nullptr;

  explicit GeneratingProject(xsmp::Generator generate,
                             std::vector<xsmp::TrackedFolder> tracked = {}) {
    xsmp::registerLanguages(*shared);
    xsmp::Contribution tool;
    tool.id = "test-tool";
    tool.generatedFolders = {"test-gen"};
    tool.trackedFolders = std::move(tracked);
    tool.generators = {std::move(generate)};
    xsmp::registry(*shared).add(std::move(tool));
    (void)directory.write("xsmp.project",
                          "project 'Generating'\nsource 'src'\ntool "
                          "'test-tool'\n");
    (void)directory.write("src/model.xsmpcat", "catalogue Model\n");
    (void)directory.write("descriptor.xsmptool", "tool 'test-tool'\n");
    cursiv::initialize_root(directory.path.string(), *shared);
    const auto document = shared->documents->getDocument(
        cursiv::path_to_file_uri((directory.path / "xsmp.project").string()));
    project = cursiv::ast_cast<xsmp::Project>(document->parseResult.value);
  }

  // Generates, and returns the failure it reports, if any.
  [[nodiscard]] std::string generate() const {
    const auto report = xsmp::generateProject(*shared, *project);
    const auto& failures = report.object().get("failures").array();
    return failures.empty()
               ? std::string{}
               : xsmp::stringAt(failures.front().object(), "message");
  }

  [[nodiscard]] bool exists(std::string_view relative) const {
    return std::filesystem::exists(directory.path / relative);
  }
};

const xsmp::TrackedFolder python{
    .generator = "gen", .output = "out", .folder = "python"};

}  // namespace

// The list is written as the other XSMP tools write it, and a renamed file
// leaves: only what generation listed, never the files of the user.
TEST(Contributions, ATrackedFolderRemovesWhatGenerationNoLongerWrites) {
  std::string name = "first";
  GeneratingProject state(
      [&](const cursiv::AstNode&, const std::filesystem::path& output) {
        xsmp::writeFileIfChanged(output / "python" / name / "__init__.py",
                                 "generated");
        xsmp::writeFileIfMissing(output / "python" / "stub.py", "stub");
      },
      {python});
  (void)state.directory.write("python/user.py", "user code");
  (void)state.directory.write("python/older/__init__.py", "not listed");
  ASSERT_EQ(state.generate(), "");
  EXPECT_EQ(
      xsmp::readFile(state.directory.path / ".xsmp/generated-files.json"),
      R"({"version":1,"outputs":[{"owner":"test-tool/gen","output":"out",)"
      R"("directory":"python","files":["first/__init__.py"]}]})"
      "\n");

  name = "second";
  ASSERT_EQ(state.generate(), "");
  EXPECT_FALSE(state.exists("python/first/__init__.py"));
  EXPECT_TRUE(state.exists("python/second/__init__.py"));
  EXPECT_TRUE(state.exists("python/user.py"));
  EXPECT_TRUE(state.exists("python/stub.py"));
  EXPECT_TRUE(state.exists("python/older/__init__.py"));
  EXPECT_NE(xsmp::readFile(state.directory.path / ".xsmp/generated-files.json")
                .find(R"("files":["second/__init__.py"])"),
            std::string::npos);
}

// A listed file that the user now owns stays, and leaves the list.
TEST(Contributions, AFileOfTheUserLeavesTheList) {
  auto mine = false;
  GeneratingProject state(
      [&](const cursiv::AstNode&, const std::filesystem::path& output) {
        const auto path = output / "python" / "shared.py";
        if (mine)
          xsmp::writeFileIfMissing(path, "stub");
        else
          xsmp::writeFileIfChanged(path, "generated");
      },
      {python});
  ASSERT_EQ(state.generate(), "");
  mine = true;
  ASSERT_EQ(state.generate(), "");
  EXPECT_TRUE(state.exists("python/shared.py"));
  EXPECT_EQ(xsmp::readFile(state.directory.path / ".xsmp/generated-files.json"),
            "{\"version\":1,\"outputs\":[]}\n");
}

// A list that cannot be read removes nothing, and is written anew.
TEST(Contributions, AnUnreadableListRemovesNothing) {
  GeneratingProject state(
      [&](const cursiv::AstNode&, const std::filesystem::path& output) {
        xsmp::writeFileIfChanged(output / "python" / "new.py", "generated");
      },
      {python});
  (void)state.directory.write(
      ".xsmp/generated-files.json",
      R"({"version":2,"outputs":[{"owner":"test-tool/gen","output":"out",)"
      R"("directory":"python","files":["old.py"]}]})");
  (void)state.directory.write("python/old.py", "listed in an unknown version");
  ASSERT_EQ(state.generate(), "");
  EXPECT_TRUE(state.exists("python/old.py"));
  EXPECT_NE(xsmp::readFile(state.directory.path / ".xsmp/generated-files.json")
                .find(R"("version":1)"),
            std::string::npos);
}

// As the other XSMP tools do.
TEST(Contributions, NothingTrackedWritesNoList) {
  GeneratingProject state(
      [&](const cursiv::AstNode&, const std::filesystem::path&) {}, {python});
  ASSERT_EQ(state.generate(), "");
  EXPECT_FALSE(state.exists(".xsmp"));
}

TEST(Contributions, ATrackedFolderInAGeneratedFolderIsRefused) {
  GeneratingProject state(
      [&](const cursiv::AstNode&, const std::filesystem::path&) {},
      {{.generator = "gen", .output = "out", .folder = "test-gen/python"}});
  EXPECT_NE(state.generate().find("is in a generated folder"),
            std::string::npos);
}

TEST(Contributions, AFailedGenerationKeepsThePreviousFiles) {
  GeneratingProject state(
      [](const cursiv::AstNode&, const std::filesystem::path&) {
        throw std::runtime_error("generation failure");
      });
  (void)state.directory.write("test-gen/previous.txt", "previous");
  const auto report = xsmp::generateProject(*state.shared, *state.project);
  ASSERT_EQ(report.object().get("failures").array().size(), 1);
  EXPECT_EQ(xsmp::readFile(state.directory.path / "test-gen/previous.txt"),
            "previous");
}

TEST(Contributions, AGenerationCancelledBeforeItStartsKeepsThePreviousFiles) {
  auto generated = false;
  GeneratingProject state(
      [&](const cursiv::AstNode&, const std::filesystem::path&) {
        generated = true;
      });
  (void)state.directory.write("test-gen/previous.txt", "previous");
  std::stop_source source;
  source.request_stop();
  EXPECT_THROW((void)xsmp::generateProject(*state.shared, *state.project, {},
                                           source.get_token()),
               cursiv::OperationCancelled);
  EXPECT_FALSE(generated);
  EXPECT_EQ(xsmp::readFile(state.directory.path / "test-gen/previous.txt"),
            "previous");
}

TEST(Contributions, AGenerationThatHasStartedGoesToItsEnd) {
  std::stop_source source;
  GeneratingProject state(
      [&](const cursiv::AstNode&, const std::filesystem::path&) {
        source.request_stop();
      });
  (void)state.directory.write("test-gen/previous.txt", "previous");
  const auto report = xsmp::generateProject(*state.shared, *state.project, {},
                                            source.get_token());
  EXPECT_EQ(report.object().get("generatedProjects").array().size(), 1u);
  EXPECT_FALSE(
      std::filesystem::exists(state.directory.path / "test-gen/previous.txt"));
}

TEST(Contributions, ASuccessfulGenerationRemovesOnlyWhatItDidNotProduce) {
  GeneratingProject state([](const cursiv::AstNode& node,
                             const std::filesystem::path& output) {
    if (cursiv::ast_cast<const xsmp::Catalogue>(&node))
      xsmp::writeFileIfChanged(output / "test-gen/kept/produced.txt", "new");
  });
  const auto folder = state.directory.path / "test-gen";
  (void)state.directory.write("test-gen/stale.txt", "stale");
  (void)state.directory.write("test-gen/old/deep/stale.txt", "stale");
  (void)state.directory.write("outside/stale.txt", "user file");
  const auto report = xsmp::generateProject(*state.shared, *state.project);
  ASSERT_EQ(report.object().get("generatedProjects").array().size(), 1);
  EXPECT_EQ(xsmp::readFile(folder / "kept/produced.txt"), "new");
  EXPECT_FALSE(std::filesystem::exists(folder / "stale.txt"));
  EXPECT_FALSE(std::filesystem::exists(folder / "old"));
  EXPECT_EQ(xsmp::readFile(state.directory.path / "outside/stale.txt"),
            "user file");
}

TEST(Contributions,
     CountsSmpErrorsInAnyVisibleSourceEvenWithNestedProjectOwner) {
  auto shared = xsmp::createXsmpSharedServices();
  xsmp::registerLanguages(*shared);
  xsmp::ScratchDirectory directory;
  xsmp::writeFileIfChanged(directory.path / "xsmp.project",
                           "project 'Outer' source '.'\n");
  xsmp::writeFileIfChanged(directory.path / "nested" / "xsmp.project",
                           "project 'Inner' source 'smdl'\n");
  xsmp::writeFileIfChanged(directory.path / "nested" / "smdl" / "broken.smpcat",
                           "<Catalogue>");
  cursiv::initialize_root(directory.path.string(), *shared);
  const auto document = shared->documents->getDocument(
      cursiv::path_to_file_uri((directory.path / "xsmp.project").string()));
  ASSERT_NE(document, nullptr);
  const auto* project =
      cursiv::ast_cast<xsmp::Project>(document->parseResult.value);
  ASSERT_NE(project, nullptr);
  const auto errors = xsmp::projectSourceErrors(*shared, *project);
  ASSERT_EQ(errors.size(), 1);
  EXPECT_EQ(
      errors.front().first,
      cursiv::path_to_file_uri(
          (directory.path / "nested" / "smdl" / "broken.smpcat").string()));
  const auto report = xsmp::generateProject(*shared, *project);
  EXPECT_TRUE(report.object().get("generatedProjects").array().empty());
  ASSERT_EQ(report.object().get("skippedProjects").array().size(), 1);
}

TEST(Contributions, ReportsSmpWarningsOfVisibleSourcesOnly) {
  auto shared = xsmp::createXsmpSharedServices();
  xsmp::registerLanguages(*shared);
  xsmp::ScratchDirectory directory;
  xsmp::writeFileIfChanged(directory.path / "App/xsmp.project",
                           "project 'App' source 'smdl'\n");
  xsmp::writeFileIfChanged(directory.path / "Other/xsmp.project",
                           "project 'Other' source 'smdl'\n");
  xsmp::writeFileIfChanged(
      directory.path / "Other/smdl/plan.smpsed",
      R"(<Schedule xmlns="http://www.ecss.nl/smp/2025/Smdl/Schedule" Name="Plan" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"><Task Name="Main"><Activity xsi:type="Schedule:Unknown"/></Task></Schedule>)");
  cursiv::initialize_root(directory.path.string(), *shared);
  const auto project = [&](const char* name) {
    const auto document =
        shared->documents->getDocument(cursiv::path_to_file_uri(
            (directory.path / name / "xsmp.project").string()));
    return cursiv::ast_cast<xsmp::Project>(document->parseResult.value);
  };
  EXPECT_TRUE(xsmp::projectSourceWarnings(*shared, *project("App")).empty());
  EXPECT_EQ(xsmp::projectSourceWarnings(*shared, *project("Other")).size(), 1);
}

using namespace xsmp;

TEST(Contributions, RepeatedNamesWithinOneContributionAreIdempotent) {
  Registry registry;
  Contribution tool;
  tool.id = "aliased";
  tool.aliases = {"same-alias", "same-alias", "aliased", ""};
  tool.deprecatedAliases = {"legacy", "legacy"};
  registry.add(std::move(tool));
  EXPECT_EQ(registry.get("same-alias").id, "aliased");
  EXPECT_EQ(registry.get("legacy").id, "aliased");
  EXPECT_EQ(registry.resolve(""), nullptr);
  Contribution collision;
  collision.id = "other";
  collision.aliases = {"same-alias"};
  EXPECT_THROW(registry.add(std::move(collision)), std::invalid_argument);
}

TEST(Contributions, RegistryResolvesAliasesAndRejectsDuplicateIds) {
  xsmp::Registry registry;
  registry.add({.id = "native",
                .label = "Native",
                .aliases = {"legacy"},
                .generators = {},
                .kind = {}});
  EXPECT_EQ(registry.get("legacy").id, "native");
  EXPECT_THROW(registry.get("missing"), std::invalid_argument);
  EXPECT_THROW(registry.add({"other", "Other", {"legacy"}, {}, {}}),
               std::invalid_argument);
  EXPECT_THROW(registry.add({"legacy", "Alias collision", {}, {}, {}}),
               std::invalid_argument);
  EXPECT_THROW(
      registry.add({"third", "Canonical collision", {"native"}, {}, {}}),
      std::invalid_argument);
  EXPECT_THROW(registry.add({"native", "Duplicate", {}, {}, {}}),
               std::invalid_argument);
}

// A host on Windows names a folder in its own spelling, which a WASI worker
// would read as one relative name.
