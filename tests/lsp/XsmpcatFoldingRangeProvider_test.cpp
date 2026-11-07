#include <gtest/gtest.h>

#include <utility>
#include <vector>

#include "support/LspTestSupport.hpp"

// Folding ranges are produced structurally from the CST for every multi-line
// brace-delimited block (namespace body, struct body). Free via the cursiv
// DefaultFoldingRangeProvider — no xsmpcat code.
TEST(XsmpcatFoldingRangeProvider, FoldingRangesForBracedBlocks) {
  auto ws = xsmp::xsmpcatWorkspace();
  (void)cursiv::test::expectFoldings(
      ws.language("xsmpcat"),
      {.text = "catalogue c\n"
               "namespace n\n"
               "{\n"
               "  struct S\n"
               "  {\n"
               "    field P x\n"
               "  }\n"
               "}\n",
       .assertion =
           [](std::span<const ::lsp::FoldingRange> ranges,
              std::span<
                  const std::pair<cursiv::TextOffset, cursiv::TextOffset>>) {
             // At least the namespace body and the struct body fold.
             EXPECT_GE(ranges.size(), 2U);
           }});
}

TEST(XsmpcatFoldingRangeProvider, FoldingStopsAtTypeBodies) {
  auto world = xsmp::xsmpcatWorkspace();
  (void)cursiv::test::expectFoldings(
      world.language("xsmpcat"),
      {.text = "catalogue c\nnamespace n\n{\n struct Payload\n {\n  "
               "field Payload x = {\n   .x = {}\n  }\n }\n}\n",
       .assertion = [](std::span<const ::lsp::FoldingRange> ranges, auto) {
         EXPECT_EQ(ranges.size(), 2u);
         EXPECT_TRUE(std::ranges::none_of(
             ranges, [](const auto& range) { return range.startLine >= 5; }));
       }});
}

TEST(XsmpcatFoldingRangeProvider,
     FoldingIncludesCommentsWithoutFoldingBlankTrivia) {
  auto world = xsmp::xsmpcatWorkspace();
  (void)cursiv::test::expectFoldings(
      world.language("xsmpcat"),
      {.text = "catalogue c\n\n\n/** First line.\n * More details.\n "
               "*/\nnamespace n\n{\n struct S {}\n}\n",
       .assertion = [](std::span<const ::lsp::FoldingRange> ranges, auto) {
         EXPECT_EQ(ranges.size(), 2u);
         EXPECT_TRUE(std::ranges::any_of(
             ranges, [](const auto& range) { return range.startLine == 3; }));
         EXPECT_FALSE(std::ranges::any_of(ranges, [](const auto& range) {
           return range.startLine == 1 || range.startLine == 2;
         }));
       }});
}
