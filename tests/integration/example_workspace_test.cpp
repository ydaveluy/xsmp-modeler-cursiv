#include <gtest/gtest.h>

#include <cursiv/utils/UriUtils.hpp>
#include <cursiv/workspace/Documents.hpp>
#include <xsmp/CliWorkspace.hpp>
#include <xsmp/workspace/SourceUri.hpp>

TEST(ExampleWorkspace, ParsesLinksAndValidatesCleanly) {
  auto shared = xsmp::createXsmpSharedServices();
  xsmp::registerLanguages(*shared);
  cursiv::initialize_root(XSMP_EXAMPLE_DIR, *shared);
  const auto prefix = cursiv::path_to_file_uri(XSMP_EXAMPLE_DIR) + "/";
  std::size_t count = 0;
  for (const auto& document : shared->documents->all()) {
    if (!xsmp::sourceDocumentUri(document->uri).starts_with(prefix)) continue;
    ++count;
    EXPECT_TRUE(document->parseSucceeded()) << document->uri;
    for (const auto& diagnostic : document->diagnostics)
      EXPECT_NE(diagnostic.severity, cursiv::DiagnosticSeverity::Error)
          << document->uri << ": " << diagnostic.message;
  }
  EXPECT_GE(count, 29u);
  EXPECT_EQ(shared->projectManager->getProjects().size(), 6u);
}
