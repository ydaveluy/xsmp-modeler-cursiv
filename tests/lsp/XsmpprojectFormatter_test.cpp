#include <gtest/gtest.h>

#include <string_view>

#include "support/LspTestSupport.hpp"

// --- Formatter -------------------------------------------------------------

// Each top-level keyword is followed by exactly one space before its string
// argument (extra spaces collapse).
TEST(XsmpprojectFormatter, CollapsesKeywordSpacing) {
  auto ws = xsmp::xsmpprojectWorkspace();
  (void)cursiv::test::expectFormatting(
      ws.language("xsmpproject"),
      {.before = "project 'P'\ndependency   'Q'\nsource  'src'\n",
       .after = "project 'P'\ndependency 'Q'\nsource 'src'\n",
       .parseOptions = xsmp::documentAt("file:///test/100/xsmp.project")});
}

// Elements crammed onto the project line are split so each keyword starts on
// its own line (Formatting.newLine on every keyword).
TEST(XsmpprojectFormatter, SplitsElementsOntoOwnLines) {
  auto ws = xsmp::xsmpprojectWorkspace();
  (void)cursiv::test::expectFormatting(
      ws.language("xsmpproject"),
      {.before = "project 'P' dependency 'Q'\n",
       .after = "project 'P'\ndependency 'Q'\n",
       .parseOptions = xsmp::documentAt("file:///test/101/xsmp.project")});
}
