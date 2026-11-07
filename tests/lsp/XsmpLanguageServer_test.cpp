#include <gtest/gtest.h>
#include <lsp/types.h>

#include <cursiv/utils/UriUtils.hpp>
#include <cursiv/workspace/WorkspaceLock.hpp>
#include <filesystem>
#include <string>
#include <xsmp/Builtins.hpp>
#include <xsmp/CliWorkspace.hpp>
#include <xsmp/SharedLspServices.hpp>
#include <xsmp/XsmpLanguageServer.hpp>
#include <xsmp/smp/SmpService.hpp>
#include <xsmp/validation/IssueCodes.hpp>
#include <xsmp/workspace/ProjectManager.hpp>
#include <xsmp/workspace/SourceUri.hpp>

#include "support/Files.hpp"
#include "support/GeneratorTestSupport.hpp"
#include "support/LspTestSupport.hpp"

namespace {

xsmp::TestWorkspace makeWorkspace() {
  auto ws = xsmp::TestWorkspace::withShared<xsmp::XsmpSharedServices>();
  auto& shared = ws.shared<xsmp::XsmpSharedServices>();
  shared.projectManager = std::make_unique<xsmp::ProjectManager>(shared);
  ws.registerLanguage(xsmp::createXsmpcatServices(shared));
  xsmp::loadXsmpcatBuiltins(shared);
  return ws;
}

// A document in no project is nothing to generate, and no error to warn about.
void expectEmptyReport(const ::lsp::LSPAny& report) {
  ASSERT_TRUE(report.isObject());
  EXPECT_TRUE(report.object().get("generatedProjects").array().empty());
  EXPECT_TRUE(report.object().get("skippedProjects").array().empty());
  EXPECT_TRUE(report.object().get("failures").array().empty());
}

}  // namespace

TEST(XsmpLanguageServer, GeneratesNothingForADocumentOutsideAnyProject) {
  auto ws = makeWorkspace();
  xsmp::XsmpLanguageServer server(ws.shared<xsmp::XsmpSharedServices>());
  auto document =
      xsmp::parse(ws, "xsmpcat", "catalogue demo\n", "file:///test.xsmpcat");
  ASSERT_TRUE(document->parseSucceeded());
  expectEmptyReport(xsmp::customRequest(server, "xsmp/generateProject",
                                        std::string("file:///test.xsmpcat")));
}

TEST(XsmpLanguageServer, GeneratesNothingForAnUnknownDocument) {
  auto ws = makeWorkspace();
  xsmp::XsmpLanguageServer server(ws.shared<xsmp::XsmpSharedServices>());
  expectEmptyReport(xsmp::customRequest(
      server, "xsmp/generateProject", std::string("file:///unknown.xsmpcat")));
}

TEST(XsmpLanguageServer, GeneratesNothingWithoutAUri) {
  auto ws = makeWorkspace();
  xsmp::XsmpLanguageServer server(ws.shared<xsmp::XsmpSharedServices>());
  expectEmptyReport(
      xsmp::customRequest(server, "xsmp/generateProject", nullptr));
}

TEST(XsmpLanguageServer, GeneratesSelectedProjectWithItsProfile) {
  auto world = xsmp::generatorWorkspace();
  xsmp::ScratchDirectory project;
  std::filesystem::create_directories(project.path / "smdl");
  const auto uri =
      cursiv::path_to_file_uri((project.path / "xsmp.project").string());
  (void)xsmp::parse(world, "xsmpproject",
                    "project 'Demo'\nsource 'smdl'\nprofile 'esa-cdk'\n", uri);
  (void)xsmp::parse(
      world, "xsmpcat",
      "catalogue Demo\nnamespace n { /** @uuid "
      "11111111-2222-4333-8444-555555555555 */ string Label[8] }\n",
      cursiv::path_to_file_uri((project.path / "smdl/Demo.xsmpcat").string()));
  xsmp::XsmpLanguageServer server(world.shared());
  const auto report =
      xsmp::customRequest(server, "xsmp/generateProject", ::lsp::LSPAny(uri));
  ASSERT_TRUE(report.isObject());
  ASSERT_EQ(report.object().get("generatedProjects").array().size(), 1u);
  EXPECT_NE(xsmp::readFile(project.path / "src-gen/n/Label.h")
                .find("::esa::ecss::smp::cdk::String<8>"),
            std::string::npos);
}

TEST(XsmpLanguageServer, WatchedFilesKeepAnSmpMirrorInStep) {
  auto world = xsmp::TestWorkspace::withShared<xsmp::XsmpSharedServices>();
  auto& shared = world.shared();
  shared.projectManager = std::make_unique<xsmp::ProjectManager>(shared);
  ASSERT_TRUE(xsmp::registerXsmpServices(shared));
  xsmp::installSmpWorkspaceManager(shared);
  xsmp::ScratchDirectory dir;
  dir.write("xsmp.project", "project 'Mirror' source '.'\n");
  const auto xml = [](std::string_view name) {
    return R"(<Configuration xmlns="http://www.ecss.nl/smp/2025/Smdl/Configuration" Name=")"
           + std::string(name) + R"("/>)";
  };
  const auto source = dir.write("model.smpcfg", xml("First"));
  const auto mirror =
      xsmp::smpMirrorUri(cursiv::path_to_file_uri(source.string()));
  cursiv::initialize_root(dir.path.string(), shared);
  ASSERT_TRUE(shared.documents->hasDocument(mirror));

  xsmp::XsmpLanguageServer server(shared);
  const auto watched = [&](const std::filesystem::path& file,
                           ::lsp::FileChangeType type) {
    ::lsp::DidChangeWatchedFilesParams params;
    params.changes.push_back(
        {.uri = ::lsp::Uri::parse(cursiv::path_to_file_uri(file.string())),
         .type = type});
    server.didChangeWatchedFiles(params);
    // Served behind the write the notification queued.
    (void)shared.workspaceLock->read([] {});
  };
  const auto mirrored = [&] {
    const auto document = shared.documents->getDocument(mirror);
    return document ? std::string(document->textDocument().getText())
                    : std::string{};
  };

  // Its XML source changes.
  EXPECT_NE(mirrored().find("First"), std::string::npos);
  dir.write("model.smpcfg", xml("Second"));
  watched(source, ::lsp::FileChangeType::Changed);
  EXPECT_NE(mirrored().find("Second"), std::string::npos);

  // A native file beside it takes its place, and gives it back when it goes.
  const auto native = dir.write("model.xsmpcfg", "configuration Native\n");
  watched(native, ::lsp::FileChangeType::Created);
  EXPECT_FALSE(shared.documents->hasDocument(mirror));
  std::filesystem::remove(native);
  watched(native, ::lsp::FileChangeType::Deleted);
  EXPECT_TRUE(shared.documents->hasDocument(mirror));

  // Its XML source goes, then comes back.
  std::filesystem::remove(source);
  watched(source, ::lsp::FileChangeType::Deleted);
  EXPECT_FALSE(shared.documents->hasDocument(mirror));
  dir.write("model.smpcfg", xml("Third"));
  watched(source, ::lsp::FileChangeType::Created);
  EXPECT_NE(mirrored().find("Third"), std::string::npos);

  // Something else that goes changes nothing.
  std::filesystem::remove(dir.path / "other.txt");
  watched(dir.path / "other.txt", ::lsp::FileChangeType::Deleted);
  EXPECT_TRUE(shared.documents->hasDocument(mirror));
}

TEST(XsmpLanguageServer, ListsProfilesAndTools) {
  auto world = xsmp::generatorWorkspace();
  xsmp::XsmpLanguageServer server(world.shared());
  const auto report =
      xsmp::customRequest(server, "xsmp/getContributionSummaries", nullptr);
  ASSERT_TRUE(report.isArray());
  EXPECT_EQ(report.array().size(), 6);
}

TEST(XsmpLanguageServer, IssuesKeepTheirCodeInTheirDataAlone) {
  auto world = xsmp::xsmpcatWorkspace();
  const auto result = cursiv::test::validationHelper(
      world.language("xsmpcat"),
      "catalogue c\nnamespace n { struct S {} field Missing x }\n");
  auto missingUuid = false;
  for (const auto& diagnostic : result.diagnostics) {
    if (cursiv::data_code(diagnostic).empty()) continue;
    missingUuid |= cursiv::data_code(diagnostic) == xsmp::MissingUuid;
    EXPECT_FALSE(diagnostic.code) << diagnostic.message;
  }
  EXPECT_TRUE(missingUuid);
}
