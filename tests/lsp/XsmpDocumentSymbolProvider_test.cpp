#include <gtest/gtest.h>

#include <algorithm>
#include <string>
#include <vector>

#include "support/LspTestSupport.hpp"

namespace {

// Depth-first collection of every symbol name in the outline tree.
void collectNames(std::span<const ::lsp::DocumentSymbol> symbols,
                  std::vector<std::string>& out) {
  for (const auto& symbol : symbols) {
    out.push_back(symbol.name);
    if (symbol.children.has_value()) {
      collectNames(*symbol.children, out);
    }
  }
}

}  // namespace

// The document-symbol outline mirrors the containment hierarchy:
// catalogue → namespace → type → members.
TEST(XsmpDocumentSymbolProvider, OutlineHierarchy) {
  auto ws = xsmp::xsmpcatWorkspace();
  (void)cursiv::test::expectSymbols(
      ws.language("xsmpcat"),
      cursiv::test::ExpectedSymbolsCallback{
          .text = "catalogue c\n"
                  "namespace n\n"
                  "{\n"
                  "  primitive P\n"
                  "  struct S\n"
                  "  {\n"
                  "    field P x\n"
                  "    field P y\n"
                  "  }\n"
                  "}\n",
          .assertion = [](std::span<const ::lsp::DocumentSymbol> symbols) {
            // The top level is the catalogue itself.
            ASSERT_EQ(symbols.size(), 1U);
            EXPECT_EQ(symbols[0].name, "c");

            std::vector<std::string> names;
            collectNames(symbols, names);
            const auto has = [&](const std::string& name) {
              return std::ranges::find(names, name) != names.end();
            };
            EXPECT_TRUE(has("n")) << "namespace missing from outline";
            EXPECT_TRUE(has("P")) << "primitive missing from outline";
            EXPECT_TRUE(has("S")) << "struct missing from outline";
            EXPECT_TRUE(has("x")) << "field x missing from outline";
            EXPECT_TRUE(has("y")) << "field y missing from outline";
          }});
}

namespace {

const ::lsp::DocumentSymbol* symbol(
    std::span<const ::lsp::DocumentSymbol> symbols, std::string_view name) {
  for (const auto& item : symbols) {
    if (item.name == name) return &item;
    if (item.children)
      if (const auto* found = symbol(*item.children, name)) return found;
  }
  return nullptr;
}

}  // namespace

TEST(XsmpDocumentSymbolProvider, DetailsDeprecatedAndParameterSignatures) {
  auto world = xsmp::xsmpcatWorkspace();
  (void)cursiv::test::expectSymbols(
      world.language("xsmpcat"),
      cursiv::test::ExpectedSymbolsCallback{
          .text =
              "catalogue Demo\nnamespace demo { public interface ILogger {} "
              "/** Legacy.\n * @deprecated Use Sensor.\n */ public model "
              "Legacy {} public model Root { container demo.Legacy* sensors "
              "reference demo.ILogger? logger public def void apply(in "
              "demo.ILogger value) } }",
          .assertion = [](const auto& symbols) {
            const auto* legacy = symbol(symbols, "Legacy");
            ASSERT_NE(legacy, nullptr);
            ASSERT_TRUE(legacy->tags);
            ASSERT_EQ(legacy->tags->size(), 1u);
            EXPECT_EQ(legacy->tags->front(), ::lsp::SymbolTag::Deprecated);
            const auto* sensors = symbol(symbols, "sensors");
            ASSERT_NE(sensors, nullptr);
            EXPECT_EQ(sensors->detail, "demo.Legacy*");
            const auto* logger = symbol(symbols, "logger");
            ASSERT_NE(logger, nullptr);
            EXPECT_EQ(logger->detail, "demo.ILogger?");
            const auto* operation =
                symbol(symbols, "apply(const demo.ILogger&)");
            ASSERT_NE(operation, nullptr);
            EXPECT_EQ(operation->detail, "void");
          }});
}

TEST(XsmpDocumentSymbolProvider, SelectionRangeIsTheName) {
  auto world = xsmp::xsmpcatWorkspace();
  const std::string text =
      "catalogue c\nnamespace n { public struct Point { field Smp.Bool x } }";
  const std::string uri = "file:///symbols/point.xsmpcat";
  (void)cursiv::test::expectSymbols(
      world.language("xsmpcat"),
      cursiv::test::ExpectedSymbolsCallback{
          .text = text,
          .assertion =
              [&](std::span<const ::lsp::DocumentSymbol> symbols) {
                ASSERT_EQ(symbols.size(), 1u);
                const auto& space = symbols[0].children->front();
                const auto& point = space.children->front();
                EXPECT_EQ(point.name, "Point");
                const auto document =
                    world.shared().documents->getDocument(uri);
                ASSERT_TRUE(document);
                const auto offset = [&](::lsp::Position position) {
                  return document->textDocument().offsetAt(
                      cursiv::from_lsp(position));
                };
                EXPECT_EQ(offset(point.range.start), text.find("public"));
                EXPECT_EQ(offset(point.selectionRange.start),
                          text.find("Point"));
                EXPECT_EQ(offset(point.selectionRange.end),
                          text.find("Point") + 5);
              },
          .parseOptions = xsmp::documentAt(uri)});
}

TEST(XsmpDocumentSymbolProvider, OutlineHoldsTheEnumerationLiterals) {
  auto world = xsmp::xsmpcatWorkspace();
  (void)cursiv::test::expectSymbols(
      world.language("xsmpcat"),
      cursiv::test::ExpectedSymbolsCallback{
          .text = "catalogue c\nnamespace n { enum E { A = 0, B = 1 } }",
          .assertion = [](std::span<const ::lsp::DocumentSymbol> symbols) {
            const auto& enumeration =
                symbols[0].children->front().children->front();
            EXPECT_EQ(enumeration.name, "E");
            ASSERT_TRUE(enumeration.children);
            ASSERT_EQ(enumeration.children->size(), 2u);
            EXPECT_EQ((*enumeration.children)[0].name, "A");
            EXPECT_EQ((*enumeration.children)[0].kind,
                      ::lsp::SymbolKind::EnumMember);
            EXPECT_EQ((*enumeration.children)[1].name, "B");
          }});
}

namespace {

std::vector<std::string> renderOutline(
    std::span<const ::lsp::DocumentSymbol> symbols, unsigned indent = 0) {
  std::vector<std::string> result;
  for (auto& symbol : symbols) {
    result.push_back(std::string(indent, ' ') + symbol.name);
    if (symbol.children) {
      auto nested = renderOutline(*symbol.children, indent + 2);
      result.insert(result.end(), nested.begin(), nested.end());
    }
  }
  return result;
}

}  // namespace

TEST(XsmpDocumentSymbolProvider,
     AssemblyOutlineIncludesConfigurationAndSubInstances) {
  auto world = xsmp::deploymentWorkspace();
  (void)cursiv::test::expectSymbols(
      world.language("xsmpasb"),
      cursiv::test::ExpectedSymbolsCallback{
          .text =
              "assembly Demo\nconfigure child/path { subscribe Start -> "
              "\"Boot\" }\nRoot: \"Impl\" { child += Child: \"ChildImpl\" { "
              "field link src.out -> dst.in } }",
          .assertion = [](const auto& symbols) {
            EXPECT_EQ(renderOutline(symbols),
                      (std::vector<std::string>{
                          "Demo", "  configure child/path",
                          "    subscribe Start -> \"Boot\"", "  Root",
                          "    child += Child",
                          "      field link src.out -> dst.in"}));
            ASSERT_TRUE(symbols[0].children);
            EXPECT_EQ(symbols[0].children->front().kind,
                      ::lsp::SymbolKind::Object);
          }});
}

TEST(XsmpDocumentSymbolProvider, LinkBaseOutlineIncludesPathsAndLinks) {
  auto world = xsmp::deploymentWorkspace();
  (void)cursiv::test::expectSymbols(
      world.language("xsmplnk"),
      cursiv::test::ExpectedSymbolsCallback{
          .text = "link Demo\n/Root { event link a -> b child { field link "
                  "local.out -> local.in } }",
          .assertion = [](const auto& symbols) {
            EXPECT_EQ(
                renderOutline(symbols),
                (std::vector<std::string>{
                    "Demo", "  /Root", "    event link a -> b", "    child",
                    "      field link local.out -> local.in"}));
          }});
}

TEST(XsmpDocumentSymbolProvider,
     ScheduleOutlineIncludesActivityArgumentsAndEvents) {
  auto world = xsmp::deploymentWorkspace();
  (void)cursiv::test::expectSymbols(
      world.language("xsmpsed"),
      cursiv::test::ExpectedSymbolsCallback{
          .text =
              "schedule Demo\ntask Main { call op(a=1i32, b=2i32) execute "
              "Other at child async emit \"Tick\" }\ntask Other {}\nevent "
              "Main mission \"PT1S\"\nevent Main on \"Boot\" until \"Stop\"",
          .assertion = [](const auto& symbols) {
            EXPECT_EQ(
                renderOutline(symbols),
                (std::vector<std::string>{
                    "Demo", "  Main", "    call op(a, b)",
                    "    execute Other at child", "    async emit \"Tick\"",
                    "  Other", "  event Main mission \"PT1S\"",
                    "  event Main on \"Boot\" until \"Stop\""}));
          }});
}

TEST(XsmpDocumentSymbolProvider,
     ConfigurationOutlineIncludesContextIncludesAndValues) {
  auto world = xsmp::deploymentWorkspace();
  (void)cursiv::test::expectSymbols(
      world.language("xsmpcfg"),
      cursiv::test::ExpectedSymbolsCallback{
          .text =
              "configuration Demo\n/local: pkg.Comp { include Other at child "
              "state = 1i32 child { value = 2i32 } }",
          .assertion = [](const auto& symbols) {
            EXPECT_EQ(
                renderOutline(symbols),
                (std::vector<std::string>{
                    "Demo", "  /local: pkg.Comp", "    include Other at child",
                    "    state", "    child", "      value"}));
          }});
}
