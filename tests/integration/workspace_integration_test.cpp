#include <gtest/gtest.h>

#include <cursiv/utils/UriUtils.hpp>
#include <cursiv/workspace/Documents.hpp>
#include <filesystem>
#include <xsmp/CliWorkspace.hpp>
#include <xsmp/workspace/SourceUri.hpp>

TEST(WorkspaceIntegration,
     SixProjectWorkspaceIncludesXmlMirrorsAndResolvesDependencies) {
  auto shared = xsmp::createXsmpSharedServices();
  xsmp::registerLanguages(*shared);
  const auto root =
      std::filesystem::path(XSMP_FIXTURES_DIR) / "workspace" / "examples";
  ASSERT_TRUE(std::filesystem::is_directory(root));
  cursiv::initialize_root(root.string(), *shared);
  const auto prefix = cursiv::path_to_file_uri(root.string()) + "/";
  std::size_t documents = 0;
  std::size_t mirrors = 0;
  for (const auto& document : shared->documents->all()) {
    if (!xsmp::sourceDocumentUri(document->uri).starts_with(prefix)) continue;
    ++documents;
    if (xsmp::isSmpMirrorUri(document->uri)) {
      ++mirrors;
      EXPECT_NE(shared->projectManager->getProject(*document), nullptr)
          << document->uri;
    }
    for (const auto& diagnostic : document->diagnostics)
      EXPECT_NE(diagnostic.severity, cursiv::DiagnosticSeverity::Error)
          << document->uri << ": " << diagnostic.message;
  }
  EXPECT_EQ(shared->projectManager->getProjects().size(), 6u);
  EXPECT_GE(documents, 29u);
  EXPECT_GE(mirrors, 1u);
}
