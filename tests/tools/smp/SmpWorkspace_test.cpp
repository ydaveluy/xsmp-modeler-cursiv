#include <gtest/gtest.h>

#include <cursiv/services/SharedCoreServices.hpp>
#include <cursiv/utils/UriUtils.hpp>
#include <cursiv/workspace/DocumentBuilder.hpp>
#include <cursiv/workspace/DocumentFactory.hpp>
#include <cursiv/workspace/Documents.hpp>
#include <cursiv/workspace/FileSystemProvider.hpp>
#include <cursiv/workspace/IndexManager.hpp>
#include <cursiv/workspace/WorkspaceManager.hpp>
#include <filesystem>
#include <pugixml.hpp>
#include <span>
#include <string>
#include <string_view>
#include <vector>
#include <xsmp/CliWorkspace.hpp>
#include <xsmp/SharedCoreServices.hpp>
#include <xsmp/ast.hpp>
#include <xsmp/smp/SmpService.hpp>
#include <xsmp/workspace/SourceUri.hpp>

#include "support/Files.hpp"

TEST(SmpWorkspace, PreloadsXmlMirrorsAndRereadsThemThroughVirtualFilesystem) {
  auto shared = xsmp::createXsmpSharedServices();
  xsmp::registerLanguages(*shared);
  xsmp::ScratchDirectory dir;
  dir.write("xsmp.project", "project 'Mirror' source '.'\n");
  auto source = dir.write(
      "model.smpcfg",
      R"(<Configuration xmlns="http://www.ecss.nl/smp/2025/Smdl/Configuration" Name="Model"><Component Path="/Root"><FieldValue xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:type="Types:Int32Value" Field="count" Value="7"/></Component></Configuration>)");
  dir.write(
      "native.smpcfg",
      R"(<Configuration xmlns="http://www.ecss.nl/smp/2025/Smdl/Configuration" Name="HiddenBySidecar"/>)");
  dir.write("native.xsmpcfg", "configuration Native\n");
  cursiv::initialize_root(dir.path.string(), *shared);
  const auto mirror =
      xsmp::smpMirrorUri(cursiv::path_to_file_uri(source.string()));
  const auto hidden = xsmp::smpMirrorUri(
      cursiv::path_to_file_uri((dir.path / "native.smpcfg").string()));
  EXPECT_TRUE(mirror.starts_with("xsmp-smp:"));
  EXPECT_TRUE(mirror.ends_with("model.smpcfg"));
  EXPECT_TRUE(shared->documents->hasDocument(mirror));
  EXPECT_FALSE(shared->documents->hasDocument(hidden));
  EXPECT_TRUE(shared->fileSystemProvider->exists(mirror));
  EXPECT_NE(shared->fileSystemProvider->readFile(mirror).find("7i32"),
            std::string::npos);
  dir.write(
      "model.smpcfg",
      R"(<Configuration xmlns="http://www.ecss.nl/smp/2025/Smdl/Configuration" Name="Changed"/>)");
  EXPECT_NE(shared->fileSystemProvider->readFile(mirror).find(
                "configuration Changed"),
            std::string::npos);
  EXPECT_FALSE(std::filesystem::exists(dir.path / "model.xsmpcfg"));
}

TEST(SmpWorkspace, XmlEditsRelinkAndHealReferencesToMirroredDocuments) {
  auto shared = xsmp::createXsmpSharedServices();
  xsmp::registerLanguages(*shared);
  xsmp::ScratchDirectory dir;
  dir.write("xsmp.project", "project 'Mirror' source '.'\n");
  dir.write(
      "shared.smpcfg",
      R"(<Configuration xmlns="http://www.ecss.nl/smp/2025/Smdl/Configuration" Name="Shared"/>)");
  auto native = shared->documentFactory->fromString(
      "configuration Native include Shared\n",
      cursiv::path_to_file_uri((dir.path / "native.xsmpcfg").string()));
  shared->documents->addDocument(native);
  auto* nativePtr = native.get();
  shared->documentBuilder->build(std::span(&nativePtr, 1),
                                 {.validation = false});
  auto* configuration =
      cursiv::ast_cast<xsmp::Configuration>(native->parseResult.value);
  ASSERT_NE(configuration, nullptr);
  auto* usage =
      cursiv::ast_cast<xsmp::ConfigurationUsage>(configuration->elements[0]);
  ASSERT_NE(usage, nullptr);
  EXPECT_EQ(usage->configuration.get(), nullptr);
  cursiv::initialize_root(dir.path.string(), *shared);
  ASSERT_NE(usage->configuration.get(), nullptr);
  EXPECT_EQ(usage->configuration.get()->name, "Shared");
  const auto mirror = xsmp::smpMirrorUri(
      cursiv::path_to_file_uri((dir.path / "shared.smpcfg").string()));
  const std::string_view changed = mirror;
  dir.write(
      "shared.smpcfg",
      R"(<Configuration xmlns="http://www.ecss.nl/smp/2025/Smdl/Configuration" Name="Changed"/>)");
  shared->documentBuilder->update(
      std::span<const std::string_view>(&changed, 1), {});
  EXPECT_EQ(usage->configuration.get(), nullptr);
  dir.write(
      "shared.smpcfg",
      R"(<Configuration xmlns="http://www.ecss.nl/smp/2025/Smdl/Configuration" Name="Shared"/>)");
  shared->documentBuilder->update(
      std::span<const std::string_view>(&changed, 1), {});
  ASSERT_NE(usage->configuration.get(), nullptr);
  EXPECT_EQ(usage->configuration.get()->name, "Shared");
}

TEST(SmpWorkspace, NativeSidecarBlocksMirroredWatchedFileReloads) {
  auto shared = xsmp::createXsmpSharedServices();
  xsmp::registerLanguages(*shared);
  xsmp::ScratchDirectory dir;
  dir.write("xsmp.project", "project 'Mirror' source '.'\n");
  auto source = dir.write(
      "model.smpcfg",
      R"(<Configuration xmlns="http://www.ecss.nl/smp/2025/Smdl/Configuration" Name="Xml"/>)");
  const auto mirror =
      xsmp::smpMirrorUri(cursiv::path_to_file_uri(source.string()));
  cursiv::initialize_root(dir.path.string(), *shared);
  EXPECT_TRUE(shared->documents->hasDocument(mirror));
  EXPECT_TRUE(shared->fileSystemProvider->exists(mirror));
  EXPECT_TRUE(shared->fileSystemProvider->stat(mirror).isFile);
  dir.write("model.xsmpcfg", "configuration Native\n");
  EXPECT_FALSE(shared->fileSystemProvider->exists(mirror));
  EXPECT_FALSE(shared->fileSystemProvider->stat(mirror).isFile);
  EXPECT_THROW(shared->fileSystemProvider->readFile(mirror),
               cursiv::FileSystemError);
  const std::string_view changed = mirror;
  shared->documentBuilder->update(
      std::span<const std::string_view>(&changed, 1), {});
  EXPECT_FALSE(shared->documents->hasDocument(changed));
  std::filesystem::remove(dir.path / "model.xsmpcfg");
  EXPECT_TRUE(shared->fileSystemProvider->exists(mirror));
  shared->documentBuilder->update(
      std::span<const std::string_view>(&changed, 1), {});
  EXPECT_TRUE(shared->documents->hasDocument(changed));
}

TEST(SmpWorkspace, FailedXmlImportsRemainVisibleUntilSuccessDeletionOrSidecar) {
  xsmp::ScratchDirectory dir;
  dir.write("xsmp.project", "project 'Mirror' source '.'\n");
  const auto file =
      dir.write("broken.smpcfg",
                "<Configuration "
                "xmlns=\"http://www.ecss.nl/smp/2025/Smdl/Configuration\">");
  auto shared = xsmp::createXsmpSharedServices();
  xsmp::registerLanguages(*shared);
  cursiv::initialize_root(dir.path.string(), *shared);
  auto errors = xsmp::sourceErrors(*shared);
  ASSERT_EQ(errors.size(), 1);
  EXPECT_TRUE(errors[0].first.ends_with("/broken.smpcfg"));
  EXPECT_NE(errors[0].second.find("Cannot parse SMP XML"), std::string::npos);
  const auto sourceUri = errors[0].first;
  const auto mirrorUri = xsmp::smpMirrorUri(sourceUri);
  dir.write(
      "broken.smpcfg",
      "<Configuration xmlns=\"http://www.ecss.nl/smp/2025/Smdl/Configuration\" "
      "Name=\"Healed\"/>");
  EXPECT_NE(shared->fileSystemProvider->readFile(mirrorUri).find("Healed"),
            std::string::npos);
  EXPECT_TRUE(xsmp::sourceErrors(*shared).empty());
  dir.write("broken.smpcfg",
            "<Configuration "
            "xmlns=\"http://www.ecss.nl/smp/2025/Smdl/Configuration\">");
  EXPECT_THROW(shared->fileSystemProvider->readFile(mirrorUri),
               cursiv::FileSystemError);
  EXPECT_EQ(xsmp::sourceErrors(*shared).size(), 1);
  dir.write("broken.xsmpcfg", "configuration Native\n");
  EXPECT_TRUE(xsmp::sourceErrors(*shared).empty());
  std::filesystem::remove(dir.path / "broken.xsmpcfg");
  EXPECT_THROW(shared->fileSystemProvider->readFile(mirrorUri),
               cursiv::FileSystemError);
  std::filesystem::remove(file);
  EXPECT_TRUE(xsmp::sourceErrors(*shared).empty());
}

TEST(SmpWorkspace, MirroredWatchEventsRespectWorkspaceAncestorExclusions) {
  xsmp::ScratchDirectory dir;
  dir.write("xsmp.project", "project 'Mirror' source '.'\n");
  auto shared = xsmp::createXsmpSharedServices();
  xsmp::registerLanguages(*shared);
  cursiv::initialize_root(dir.path.string(), *shared);
  const auto included = [&](std::string_view relative) {
    const auto source =
        cursiv::path_to_file_uri((dir.path / relative).string());
    return shared->workspaceManager->isIncluded(
        {.isFile = true,
         .isDirectory = false,
         .uri = xsmp::smpMirrorUri(source)});
  };
  EXPECT_TRUE(included("model.smpcfg"));
  EXPECT_TRUE(included("src/nested/model.smpcat"));
  EXPECT_TRUE(included("build/model.smpcat"));
  EXPECT_TRUE(included("src-gen/model.smpcat"));
  EXPECT_TRUE(included("lib/model.smpcat"));
  EXPECT_TRUE(included("coverage/model.smpcat"));
  for (const auto* directory : {".cache", "node_modules", "out", "smdl-gen"}) {
    SCOPED_TRACE(directory);
    EXPECT_FALSE(included(std::string(directory) + "/model.smpcat"));
  }
  EXPECT_TRUE(shared->workspaceManager->isIncluded(
      {.isFile = true,
       .isDirectory = false,
       .uri = cursiv::path_to_file_uri(
           (dir.path / "lib/model.xsmpcat").string())}));
}

TEST(SmpWorkspace, ClosingWorkspaceFoldersDropsSourceErrorsStillPresentOnDisk) {
  xsmp::ScratchDirectory dir;
  dir.write("xsmp.project", "project 'Mirror' source '.'\n");
  const auto source =
      dir.write("broken.smpcfg",
                "<Configuration "
                "xmlns=\"http://www.ecss.nl/smp/2025/Smdl/Configuration\">");
  auto shared = xsmp::createXsmpSharedServices();
  xsmp::registerLanguages(*shared);
  cursiv::initialize_root(dir.path.string(), *shared);
  ASSERT_EQ(xsmp::sourceErrors(*shared).size(), 1u);
  const auto mirror =
      xsmp::smpMirrorUri(cursiv::path_to_file_uri(source.string()));
  shared->workspaceManager->initializeWorkspace({});
  EXPECT_TRUE(std::filesystem::exists(source));
  EXPECT_TRUE(xsmp::sourceErrors(*shared).empty());
  // A late virtual-document request from the removed folder must not
  // resurrect its diagnostic in the active workspace's error count.
  EXPECT_THROW(shared->fileSystemProvider->readFile(mirror),
               cursiv::FileSystemError);
  EXPECT_TRUE(xsmp::sourceErrors(*shared).empty());
}

TEST(SmpWorkspace, ImportWarningsRemainOnXmlSourcesAndClearWithLifecycle) {
  xsmp::ScratchDirectory dir;
  dir.write("xsmp.project", "project 'Mirror' source '.'\n");
  const std::string unsupported =
      R"(<Schedule xmlns="http://www.ecss.nl/smp/2025/Smdl/Schedule" Name="Plan" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"><Task Name="Main"><Activity xsi:type="Schedule:Unknown"/></Task></Schedule>)";
  const auto source = dir.write("plan.smpsed", unsupported);
  auto shared = xsmp::createXsmpSharedServices();
  xsmp::registerLanguages(*shared);
  cursiv::initialize_root(dir.path.string(), *shared);
  auto warnings = xsmp::sourceWarnings(*shared);
  ASSERT_EQ(warnings.size(), 1u);
  EXPECT_EQ(warnings[0].first, cursiv::path_to_file_uri(source.string()));
  EXPECT_EQ(warnings[0].second,
            "Unsupported schedule activity type 'Schedule:Unknown'.");
  EXPECT_TRUE(xsmp::sourceErrors(*shared).empty());
  const auto mirror = xsmp::smpMirrorUri(warnings[0].first);
  dir.write("plan.smpsed",
            "<Schedule xmlns=\"http://www.ecss.nl/smp/2025/Smdl/Schedule\" "
            "Name=\"Plan\"/>");
  (void)shared->fileSystemProvider->readFile(mirror);
  EXPECT_TRUE(xsmp::sourceWarnings(*shared).empty());
  dir.write("plan.smpsed", unsupported);
  (void)shared->fileSystemProvider->readFile(mirror);
  EXPECT_EQ(xsmp::sourceWarnings(*shared).size(), 1u);
  dir.write("plan.xsmpsed", "schedule Native\n");
  warnings = xsmp::sourceWarnings(*shared);
  ASSERT_EQ(warnings.size(), 1u);
  EXPECT_EQ(warnings[0].second,
            "Ignoring 'plan.smpsed' because 'plan.xsmpsed' is present in the "
            "same source folder.");
  std::filesystem::remove(dir.path / "plan.xsmpsed");
  (void)shared->fileSystemProvider->readFile(mirror);
  EXPECT_EQ(xsmp::sourceWarnings(*shared).size(), 1u);
  shared->workspaceManager->initializeWorkspace({});
  EXPECT_TRUE(xsmp::sourceWarnings(*shared).empty());
}

TEST(SmpWorkspace,
     AutomaticMirrorsFollowProjectSourcesAndRemoveInvalidDocuments) {
  xsmp::ScratchDirectory dir;
  const auto project =
      dir.write("xsmp.project", "project 'Sources' source 'src'\n");
  const auto catalogue = [](std::string_view name) {
    return "<Catalogue xmlns=\"http://www.ecss.nl/smp/2025/Smdl/Catalogue\" "
           "Name=\"" +
           std::string(name) +
           "\" "
           "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"><Namespace "
           "Name=\"n\"><Type xsi:type=\"Types:Structure\" Name=\"" +
           std::string(name) +
           "\" Visibility=\"public\"/></Namespace></Catalogue>";
  };
  const auto first = dir.write("src/first.smpcat", catalogue("First"));
  const auto second = dir.write("other/second.smpcat", catalogue("Second"));
  auto shared = xsmp::createXsmpSharedServices();
  xsmp::registerLanguages(*shared);
  cursiv::initialize_root(dir.path.string(), *shared);
  const auto hasSymbol = [&](std::string_view name) {
    auto found = false;
    (void)shared->indexManager->visitElements(
        nullptr, {}, [&](const auto& description) {
          found |= description.name == name;
          return true;
        });
    return found;
  };
  const auto firstMirror =
      xsmp::smpMirrorUri(cursiv::path_to_file_uri(first.string()));
  const auto secondMirror =
      xsmp::smpMirrorUri(cursiv::path_to_file_uri(second.string()));
  const auto update = [&](const std::string& uri) {
    const std::string_view view(uri);
    shared->documentBuilder->update(std::span(&view, 1), {});
  };
  EXPECT_TRUE(hasSymbol("n.First"));
  EXPECT_FALSE(hasSymbol("n.Second"));
  EXPECT_EQ(xsmp::takeMirrorChanges(*shared).changed,
            (std::vector<std::string>{firstMirror}));
  update(secondMirror);
  EXPECT_FALSE(shared->documents->hasDocument(secondMirror));
  EXPECT_FALSE(hasSymbol("n.Second"));
  dir.write("xsmp.project", "project 'Sources' source 'other'\n");
  update(cursiv::path_to_file_uri(project.string()));
  EXPECT_FALSE(hasSymbol("n.First"));
  EXPECT_TRUE(hasSymbol("n.Second"));
  EXPECT_FALSE(shared->documents->hasDocument(firstMirror));
  dir.write("other/second.smpcat",
            "<Catalogue xmlns=\"http://www.ecss.nl/smp/2025/Smdl/Catalogue\">");
  update(secondMirror);
  EXPECT_FALSE(hasSymbol("n.Second"));
  EXPECT_FALSE(shared->documents->hasDocument(secondMirror));
  ASSERT_EQ(xsmp::sourceErrors(*shared).size(), 1u);
  dir.write("other/second.smpcat", catalogue("Second"));
  update(secondMirror);
  EXPECT_TRUE(hasSymbol("n.Second"));
  EXPECT_TRUE(xsmp::sourceErrors(*shared).empty());
  dir.write("xsmp.project", "project 'Sources' source 'src' source 'other'\n");
  update(cursiv::path_to_file_uri(project.string()));
  EXPECT_TRUE(hasSymbol("n.First"));
  EXPECT_TRUE(hasSymbol("n.Second"));
}

// An XML file that no source root holds and no link names, such as one that
// generation writes, cannot change a mirror.
TEST(SmpWorkspace, OnlyTheXmlFilesThatCanMoveAMirrorRefreshThem) {
  xsmp::ScratchDirectory dir;
  dir.write("xsmp.project", "project 'Mirror' source 'smdl'\n");
  const auto xml = [](std::string_view name, std::string_view include = {}) {
    std::string text =
        R"(<Configuration:Configuration xmlns:Configuration="http://www.ecss.nl/smp/2025/Smdl/Configuration" xmlns:xlink="http://www.w3.org/1999/xlink" Name=")"
        + std::string(name) + R"(">)";
    if (!include.empty())
      text += R"(<Include><Configuration xlink:href=")" + std::string(include)
              + R"(#lib"/></Include>)";
    return text + "</Configuration:Configuration>";
  };
  const auto source =
      dir.write("smdl/model.smpcfg", xml("First", "lib.smpcfg"));
  auto shared = xsmp::createXsmpSharedServices();
  xsmp::registerLanguages(*shared);
  cursiv::initialize_root(dir.path.string(), *shared);
  const auto mirror =
      xsmp::smpMirrorUri(cursiv::path_to_file_uri(source.string()));
  (void)xsmp::takeMirrorChanges(*shared);
  const auto updated = [&](const std::filesystem::path& file) {
    const auto uri = cursiv::path_to_file_uri(file.string());
    const std::string_view view(uri);
    shared->documentBuilder->update(std::span(&view, 1), {});
    return xsmp::takeMirrorChanges(*shared).changed;
  };
  // A refresh would see each new name, so no change means that none ran.
  dir.write("smdl/model.smpcfg", xml("Second", "lib.smpcfg"));
  EXPECT_TRUE(updated(dir.write("smdl-gen/out.smpcfg", xml("Out"))).empty());
  EXPECT_TRUE(updated(dir.write("build/out.smpcfg", xml("Out"))).empty());
  EXPECT_TRUE(updated(dir.write("smdl/.hidden.smpcfg", xml("Out"))).empty());
  // A file of the name that a link looks for, wherever it is.
  EXPECT_EQ(updated(dir.write("build/lib.smpcfg", xml("Lib"))),
            (std::vector<std::string>{mirror}));
  // Any XML file of a source root.
  dir.write("smdl/model.smpcfg", xml("Third", "lib.smpcfg"));
  EXPECT_EQ(updated(dir.write("smdl/other.smpcfg", xml("Other"))).size(), 2u);
}

TEST(SmpWorkspace, EditingNativeDocumentsLeavesTheMirrorsAlone) {
  xsmp::ScratchDirectory dir;
  dir.write("xsmp.project", "project 'Mirror' source '.'\n");
  const auto xml = [](std::string_view name) {
    return R"(<Configuration xmlns="http://www.ecss.nl/smp/2025/Smdl/Configuration" Name=")"
           + std::string(name) + R"("/>)";
  };
  const auto source = dir.write("model.smpcfg", xml("First"));
  const auto other = dir.write("other.xsmpcfg", "configuration Other\n");
  const auto beside = dir.path / "model.xsmpcfg";
  auto shared = xsmp::createXsmpSharedServices();
  xsmp::registerLanguages(*shared);
  cursiv::initialize_root(dir.path.string(), *shared);
  const auto mirror =
      xsmp::smpMirrorUri(cursiv::path_to_file_uri(source.string()));
  (void)xsmp::takeMirrorChanges(*shared);
  const auto updated = [&](const std::filesystem::path& file) {
    const auto uri = cursiv::path_to_file_uri(file.string());
    const std::string_view view(uri);
    shared->documentBuilder->update(std::span(&view, 1), {});
    return xsmp::takeMirrorChanges(*shared);
  };
  // A refresh would see this new name, so no change means that none ran.
  dir.write("model.smpcfg", xml("Second"));
  EXPECT_TRUE(updated(other).changed.empty());
  EXPECT_TRUE(updated(beside).changed.empty());

  // A native file that appears beside the XML file shadows it.
  dir.write("model.xsmpcfg", "configuration Native\n");
  EXPECT_EQ(updated(beside).deleted, (std::vector<std::string>{mirror}));
  EXPECT_FALSE(shared->documents->hasDocument(mirror));

  // Editing it changes nothing more, whatever the XML file now says.
  dir.write("model.smpcfg", "<Configuration");
  dir.write("model.xsmpcfg", "configuration Edited\n");
  const auto edited = updated(beside);
  EXPECT_TRUE(edited.changed.empty());
  EXPECT_TRUE(edited.deleted.empty());

  // Deleting it brings the mirror back.
  dir.write("model.smpcfg", xml("Third"));
  std::filesystem::remove(beside);
  const auto uri = cursiv::path_to_file_uri(beside.string());
  const std::string_view view(uri);
  shared->documentBuilder->update({}, std::span(&view, 1));
  EXPECT_EQ(xsmp::takeMirrorChanges(*shared).changed,
            (std::vector<std::string>{mirror}));
  EXPECT_TRUE(shared->documents->hasDocument(mirror));
}
