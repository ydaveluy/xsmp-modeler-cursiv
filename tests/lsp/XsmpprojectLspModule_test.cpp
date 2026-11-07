#include <gtest/gtest.h>

#include <string_view>

#include "support/LspTestSupport.hpp"

// --- Hover -------------------------------------------------------------------

// xsmpproject has no custom hover provider — it relies on cursiv's default
// `MultilineCommentHoverProvider`, which (like xsmpcat's) dispatches a keyword
// CST position to the documentation attached via `"kw"_kw.doc("…")`.
TEST(XsmpprojectLspModule, RendersKeywordDocumentation) {
  auto ws = xsmp::xsmpprojectWorkspace();
  (void)cursiv::test::expectHover(
      ws.language("xsmpproject"),
      {.text = "proj<|>ect 'demo'\n",
       .hoverMatches = cursiv::test::contains("XSMP")});
}
