#include <gtest/gtest.h>

#include <cursiv/workspace/Documents.hpp>
#include <xsmp/XsmpTextDocumentContentProvider.hpp>

#include "support/LspTestSupport.hpp"

TEST(XsmpTextDocumentContentProvider,
     TextDocumentContentReadsAnySpellingOfADocument) {
  auto world = xsmp::xsmpcatWorkspace();
  auto& shared = world.shared();
  const xsmp::XsmpTextDocumentContentProvider provider(shared);
  const auto content = [&](std::string_view uri) {
    ::lsp::TextDocumentContentParams params;
    params.uri = ::lsp::DocumentUri::parse(std::string(uri));
    return provider.provideTextDocumentContent(params);
  };
  (void)shared.documents->createDocument("xsmp:/probe%40ECSS_SMP_2025.xsmpcat",
                                         "catalogue Probe");
  for (const std::string_view spelled :
       {"xsmp:/probe%40ECSS_SMP_2025.xsmpcat",
        "xsmp:///probe@ECSS_SMP_2025.xsmpcat"}) {
    const auto result = content(spelled);
    ASSERT_TRUE(result) << spelled;
    EXPECT_EQ(result->text, "catalogue Probe") << spelled;
  }
  EXPECT_FALSE(content("xsmp:/unknown.xsmpcat"));
  EXPECT_FALSE(content("file:///elsewhere/unknown.xsmpcat"));
}
