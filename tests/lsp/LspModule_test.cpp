#include <algorithm>
#include <array>
#include <cstdint>
#include <cursiv/hover/HoverProvider.hpp>
#include <cursiv/navigation/DefinitionProvider.hpp>
#include <cursiv/navigation/ReferencesProvider.hpp>
#include <cursiv/navigation/RenameProvider.hpp>
#include <filesystem>
#include <variant>
#include <xsmp/Builtins.hpp>
#include <xsmp/Json.hpp>

#include "support/LspTestSupport.hpp"

TEST(LspModule, FourLanguageProvidersAreInstalled) {
  auto world = xsmp::deploymentWorkspace();
  for (const auto id : {"xsmpasb", "xsmpcfg", "xsmplnk", "xsmpsed"}) {
    EXPECT_NE(world.language(id).formatter, nullptr);
    EXPECT_NE(world.language(id).completionProvider, nullptr);
    EXPECT_NE(world.language(id).definitionProvider, nullptr);
    EXPECT_NE(world.language(id).renameProvider, nullptr);
  }
}

namespace {

using CoordinateRange = std::array<std::uint64_t, 4>;

CoordinateRange coordinates(const ::lsp::Range& range) {
  return {range.start.line, range.start.character, range.end.line,
          range.end.character};
}

CoordinateRange coordinates(const xsmp::Json& value) {
  const auto& object = value.object();
  const auto& start = object.get("start").object();
  const auto& end = object.get("end").object();
  return {static_cast<std::uint64_t>(start.get("line").integer()),
          static_cast<std::uint64_t>(start.get("character").integer()),
          static_cast<std::uint64_t>(end.get("line").integer()),
          static_cast<std::uint64_t>(end.get("character").integer())};
}

std::string locationKey(std::string_view uri, CoordinateRange range,
                        std::string_view newText = {}) {
  std::string key(uri);
  for (auto value : range) key += ":" + std::to_string(value);
  return key + ":" + std::string(newText);
}

}  // namespace

TEST(LspModule, NavigationHoverPrepareReferencesAndRename) {
  using namespace xsmp;
  const auto records =
      ::lsp::json::parse(readText(std::filesystem::path(XSMP_FIXTURES_DIR)
                                  / "lsp/deployment-navigation-services.json"));
  const auto& cases = records.object().get("cases").array();
  ASSERT_EQ(cases.size(), 25u);
  for (const auto& entry : cases) {
    const auto& record = entry.object();
    SCOPED_TRACE(stringAt(record, "name"));
    auto world = xsmp::deploymentWorkspace();
    world.registerLanguage(xsmp::createXsmpcatServices(world.shared()));
    world.registerLanguage(xsmp::createXsmpprojectServices(world.shared()));
    xsmp::loadXsmpcatBuiltins(world.shared());
    for (const auto& background : record.get("documents").array()) {
      const auto uri = stringAt(background.object(), "uri");
      const auto language =
          uri.ends_with("xsmp.project")
              ? std::string("xsmpproject")
              : std::filesystem::path(uri).extension().string().substr(1);
      (void)xsmp::parse(world, language, stringAt(background.object(), "text"),
                        uri);
    }
    const auto languageId = stringAt(record, "language");
    auto input = xsmp::marked(world, languageId, stringAt(record, "text"),
                              stringAt(record, "uri"));
    const auto& language = world.language(languageId);
    const auto& expected = record.get("expected").object();

    const auto definition = language.definitionProvider->getDefinition(
        *input.document, input.position);
    const auto& expectedDefinition = expected.get("textDocument/definition");
    const auto expectedCount =
        expectedDefinition.isArray() ? expectedDefinition.array().size() : 0u;
    ASSERT_EQ(definition ? definition->size() : 0u, expectedCount);
    for (std::size_t i = 0; i < expectedCount; ++i) {
      const auto& actual = (*definition)[i];
      const auto& target = expectedDefinition.array()[i].object();
      EXPECT_EQ(actual.targetUri.toString(), stringAt(target, "targetUri"));
      EXPECT_EQ(coordinates(actual.targetRange),
                coordinates(target.get("targetRange")));
      EXPECT_EQ(coordinates(actual.targetSelectionRange),
                coordinates(target.get("targetSelectionRange")));
      ASSERT_TRUE(actual.originSelectionRange);
      EXPECT_EQ(coordinates(*actual.originSelectionRange),
                coordinates(target.get("originSelectionRange")));
    }

    const auto hover = language.hoverProvider->getHoverContent(*input.document,
                                                               input.position);
    const auto& expectedHover = expected.get("textDocument/hover");
    ASSERT_EQ(hover.has_value(), expectedHover.isObject());
    if (hover) {
      const auto* markup = std::get_if<::lsp::MarkupContent>(&hover->contents);
      ASSERT_NE(markup, nullptr);
      EXPECT_EQ(markup->kind, ::lsp::MarkupKind::Markdown);
      EXPECT_EQ(
          markup->value,
          stringAt(expectedHover.object().get("contents").object(), "value"));
    }

    const auto prepared =
        language.renameProvider->prepareRename(*input.document, input.position);
    const auto& expectedPrepare = expected.get("textDocument/prepareRename");
    ASSERT_EQ(prepared.has_value(), expectedPrepare.isObject());
    if (prepared)
      EXPECT_EQ(coordinates(*prepared), coordinates(expectedPrepare));

    std::vector<std::string> references, expectedReferences;
    for (const auto& location : language.referencesProvider->findReferences(
             *input.document, input.position, true))
      references.push_back(
          locationKey(location.uri.toString(), coordinates(location.range)));
    for (const auto& location : expected.get("textDocument/references").array())
      expectedReferences.push_back(
          locationKey(stringAt(location.object(), "uri"),
                      coordinates(location.object().get("range"))));
    std::ranges::sort(references);
    std::ranges::sort(expectedReferences);
    EXPECT_EQ(references, expectedReferences);

    // Rename runs last.
    const auto renamed = language.renameProvider->rename(
        *input.document, input.position, "Renamed");
    const auto& expectedRename = expected.get("textDocument/rename");
    ASSERT_EQ(renamed.has_value(), expectedRename.isObject());
    if (renamed) {
      ASSERT_TRUE(renamed->changes);
      std::vector<std::string> edits, expectedEdits;
      for (const auto& [uri, values] : *renamed->changes)
        for (const auto& value : values)
          edits.push_back(locationKey(uri.toString(), coordinates(value.range),
                                      value.newText));
      for (const auto& [uri, values] :
           expectedRename.object().get("changes").object())
        for (const auto& value : values.array())
          expectedEdits.push_back(
              locationKey(uri, coordinates(value.object().get("range")),
                          stringAt(value.object(), "newText")));
      std::ranges::sort(edits);
      std::ranges::sort(expectedEdits);
      EXPECT_EQ(edits, expectedEdits);
    }
  }
}

TEST(LspModule, KeywordHoverMarkdownAcrossLanguages) {
  using namespace xsmp;
  const auto records =
      ::lsp::json::parse(readText(std::filesystem::path(XSMP_FIXTURES_DIR)
                                  / "lsp/keyword-hover-contracts.json"));
  for (const auto& entry : records.object().get("cases").array()) {
    const auto& record = entry.object();
    SCOPED_TRACE(stringAt(record, "language"));
    auto world = xsmp::deploymentWorkspace();
    world.registerLanguage(xsmp::createXsmpcatServices(world.shared()));
    world.registerLanguage(xsmp::createXsmpprojectServices(world.shared()));
    const auto language = stringAt(record, "language");
    const auto document =
        xsmp::parse(world, language, stringAt(record, "text"));
    const auto& position = record.get("position").object();
    const auto hover = world.language(language).hoverProvider->getHoverContent(
        *document,
        {.line = static_cast<std::uint32_t>(position.get("line").integer()),
         .character =
             static_cast<std::uint32_t>(position.get("character").integer())});
    ASSERT_TRUE(hover);
    const auto* markup = std::get_if<::lsp::MarkupContent>(&hover->contents);
    ASSERT_NE(markup, nullptr);
    EXPECT_EQ(markup->kind, ::lsp::MarkupKind::Markdown);
    EXPECT_EQ(markup->value,
              stringAt(record.get("expected").object().get("contents").object(),
                       "value"));
  }
}
