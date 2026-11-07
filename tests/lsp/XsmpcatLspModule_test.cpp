#include <gtest/gtest.h>

#include <cstddef>
#include <optional>
#include <string>
#include <vector>
#include <xsmp/Builtins.hpp>

#include "support/LspTestSupport.hpp"

// The hover shows the doc comment, without a computed-value suffix.
TEST(XsmpcatLspModule, RendersDocCommentWithoutInventingAComputedValue) {
  auto ws = xsmp::xsmpcatWorkspace();
  (void)cursiv::test::expectHover(
      ws.language("xsmpcat"), {.text = "catalogue c\n"
                                       "namespace n\n"
                                       "{\n"
                                       "  primitive P\n"
                                       "  struct S\n"
                                       "  {\n"
                                       "    /** The answer to everything. */\n"
                                       "    constant P Ans<|>wer = 42\n"
                                       "  }\n"
                                       "}\n",
                               .hover = "The answer to everything."});
}

// Hovering over a grammar keyword (not an AST declaration) renders the
// documentation attached via `"kw"_kw.doc("…")` in the grammar — cursiv's
// `AstNodeHoverProvider` dispatches a keyword CST position to this before
// falling back to cross-reference resolution.
TEST(XsmpcatLspModule, RendersKeywordDocumentation) {
  auto ws = xsmp::xsmpcatWorkspace();
  (void)cursiv::test::expectHover(ws.language("xsmpcat"),
                                  {.text = "cat<|>alogue c\n",
                                   .hoverMatches = cursiv::test::contains(
                                       "Declares a catalogue document.")});
}

// A cross-reference to a real builtin (`Smp.Char8`) whose doc comment has a
// leading description PARAGRAPH, then a blank comment line, then `@uuid`/
// `@id` tags — the exact shape used throughout the real ECSS builtin corpus
// — renders BOTH the description and the tags on hover, not just the tags.
// Regression guard: `DocComment`'s parser routes the blank-line break into
// the description paragraph's own children (`append_empty_line`) rather than
// dropping the paragraph, so `DocComment::toMarkdown` still emits it before
// the following `@uuid`/`@id` tag blocks.
TEST(XsmpcatLspModule, DescriptionBeforeUuidRendersOnCrossReferenceHover) {
  auto ws = xsmp::xsmpcatWorkspace();
  xsmp::loadXsmpcatBuiltins(ws.shared<xsmp::XsmpSharedServices>());
  (void)cursiv::test::expectHover(
      ws.language("xsmpcat"),
      {.text = "catalogue c\n"
               "namespace n\n"
               "{\n"
               "  struct S { field Smp.Char<|>8 x }\n"
               "}\n",
       .hoverMatches = [](std::string_view text) {
         return text.find("8 bit character") != std::string_view::npos
                && text.find("204368617238") != std::string_view::npos;
       }});
}

TEST(XsmpcatLspModule, RendersStructureKeywordDocumentation) {
  auto ws = xsmp::xsmpcatWorkspace();
  (void)cursiv::test::expectHover(ws.language("xsmpcat"),
                                  {.text = "catalogue c\n"
                                           "namespace n { str<|>uct S {} }\n",
                                   .hoverMatches = cursiv::test::contains(
                                       "Defines a structured value type")});
}

TEST(XsmpcatLspModule, DoesNotInventUndocumentedConstantHovers) {
  auto ws = xsmp::xsmpcatWorkspace();
  xsmp::loadXsmpcatBuiltins(ws.shared());
  (void)cursiv::test::expectHover(
      ws.language("xsmpcat"),
      {.text = "catalogue c\nnamespace n { struct S { constant Smp.UInt64 "
               "M<|>ax = 18446744073709551615UL } }\n"});
}

// Find-references from a type declaration returns its use sites (the field
// type reference). `<|>` marks the cursor on the declaration; `<|P|>` marks the
// expected reference occurrence.
TEST(XsmpcatLspModule, FindReferencesFromDeclaration) {
  auto ws = xsmp::xsmpcatWorkspace();
  (void)cursiv::test::expectFindReferences(
      ws.language("xsmpcat"), {
                                  .text = "catalogue c\n"
                                          "namespace n\n"
                                          "{\n"
                                          "  primitive <|>P\n"
                                          "  struct S { field <|P|> x }\n"
                                          "}\n",
                                  .includeDeclaration = false,
                              });
}

// Document highlight marks every occurrence of the symbol under the cursor:
// the declaration (range 0) and the field-type reference (range 1). The cursor
// `<|>` is placed on the declaration.
TEST(XsmpcatLspModule, HighlightMarksAllOccurrences) {
  auto ws = xsmp::xsmpcatWorkspace();
  (void)cursiv::test::expectHighlight(
      ws.language("xsmpcat"), {
                                  .text = "catalogue c\n"
                                          "namespace n\n"
                                          "{\n"
                                          "  primitive <|><|P|>\n"
                                          "  struct S { field <|P|> x }\n"
                                          "}\n",
                                  .rangeIndex = std::vector<std::size_t>{0, 1},
                              });
}
