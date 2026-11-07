#include <gtest/gtest.h>

#include <cstddef>
#include <cursiv/navigation/DefinitionProvider.hpp>
#include <cursiv/navigation/DocumentHighlightProvider.hpp>
#include <cursiv/navigation/TypeDefinitionProvider.hpp>
#include <cursiv/references/References.hpp>
#include <filesystem>
#include <vector>
#include <xsmp/Builtins.hpp>
#include <xsmp/Json.hpp>

#include "support/LspTestSupport.hpp"

TEST(XsmpDefinitionProvider, TemplateParameterDefinitionInsidePathToken) {
  auto world = xsmp::deploymentWorkspace();
  auto input = xsmp::marked(world, "xsmpsed",
                            "schedule <Root: string = \"child\"> "
                            "Demo\ntask Main { trig {Ro<|>ot}.step }",
                            "file:///outline/template.xsmpsed");
  auto result = world.language("xsmpsed").definitionProvider->getDefinition(
      *input.document, input.position);
  ASSERT_TRUE(result);
  ASSERT_EQ(result->size(), 1u);
  auto target = result->front().targetSelectionRange;
  EXPECT_EQ(target.start.line, 0u);
  EXPECT_EQ(target.start.character, 10u);
  EXPECT_EQ(target.end.character, 14u);
}

TEST(XsmpDefinitionProvider, PathAndTemplateDefinitionContracts) {
  using namespace xsmp;
  const auto records = ::lsp::json::parse(
      readText(std::filesystem::path(XSMP_FIXTURES_DIR)
               / "lsp/deployment-navigation-contracts.json"));
  const auto* cases = records.object().find("cases");
  ASSERT_TRUE(cases && cases->isArray());
  ASSERT_EQ(cases->array().size(), 22u);
  for (const auto& entry : cases->array()) {
    const auto& record = entry.object();
    SCOPED_TRACE(stringAt(record, "name"));
    auto world = xsmp::deploymentWorkspace();
    world.registerLanguage(xsmp::createXsmpcatServices(world.shared()));
    world.registerLanguage(xsmp::createXsmpprojectServices(world.shared()));
    xsmp::loadXsmpcatBuiltins(world.shared());
    for (const auto& background : record.find("documents")->array()) {
      const auto uri = stringAt(background.object(), "uri");
      const auto language =
          uri.ends_with("xsmp.project")
              ? std::string("xsmpproject")
              : std::filesystem::path(uri).extension().string().substr(1);
      (void)xsmp::parse(world, language, stringAt(background.object(), "text"),
                        uri);
    }
    auto input =
        xsmp::marked(world, stringAt(record, "language"),
                     stringAt(record, "text"), stringAt(record, "uri"));
    const auto result =
        world.language(stringAt(record, "language"))
            .definitionProvider->getDefinition(*input.document, input.position);
    const auto& targets = record.find("targets")->array();
    ASSERT_EQ(result ? result->size() : 0u, targets.size());
    for (std::size_t i = 0; i < targets.size(); ++i) {
      const auto& actual = (*result)[i];
      const auto& expected = targets[i].object();
      EXPECT_EQ(actual.targetUri.toString(), stringAt(expected, "uri"));
      const auto target =
          world.shared().documents->getDocument(stringAt(expected, "uri"));
      ASSERT_TRUE(target);
      const auto range = actual.targetSelectionRange;
      const auto begin = target->textDocument().offsetAt(
                     cursiv::from_lsp(range.start)),
                 end = target->textDocument().offsetAt(
                     cursiv::from_lsp(range.end));
      EXPECT_EQ(target->textDocument().getText().substr(begin, end - begin),
                stringAt(expected, "name"));
    }
  }
}

// Go-to-definition on a Type cross-reference resolves to the type's
// declaration. Works out of the box: the DefaultDefinitionProvider uses the
// reference index already produced by the xsmpcat linker. `<|P|>` marks the
// expected target range (index 0); `<|>` marks the cursor on the reference.
TEST(XsmpDefinitionProvider, GoToDefinitionResolvesTypeReference) {
  auto ws = xsmp::xsmpcatWorkspace();
  (void)cursiv::test::expectGoToDefinition(
      ws.language("xsmpcat"), {
                                  .text = "catalogue c\n"
                                          "namespace n\n"
                                          "{\n"
                                          "  primitive <|P|>\n"
                                          "  struct S { field <|>P x }\n"
                                          "}\n",
                                  .rangeIndex = std::size_t{0},
                              });
}

TEST(XsmpDefinitionProvider, GoToTypeDefinitionIsAdvertisedAndResolvesType) {
  auto world = xsmp::xsmpcatWorkspace();
  auto input =
      xsmp::marked(world, "xsmpcat",
                   "catalogue c\nnamespace n { public struct Payload {} "
                   "model M { field Pay<|>load value } }");
  ASSERT_NE(world.language("xsmpcat").typeDefinitionProvider, nullptr);
  auto result =
      world.language("xsmpcat").typeDefinitionProvider->getTypeDefinition(
          *input.document, input.position);
  ASSERT_TRUE(result);
  ASSERT_EQ(result->size(), 1u);
  EXPECT_EQ(result->front().targetSelectionRange.start.line, 1u);
  EXPECT_EQ(result->front().targetSelectionRange.end.character
                - result->front().targetSelectionRange.start.character,
            7u);
}

TEST(XsmpDefinitionProvider, GoToTypeDefinitionOnAMemberReachesItsType) {
  auto world = xsmp::xsmpcatWorkspace();
  const std::string text =
      "catalogue c\nnamespace n\n{\n"
      "  public enum E { A = 0 }\n"
      "  public struct T { field E f constant E k = n.E.A }\n"
      "  public interface I {}\n"
      "  public model M { reference I* r def void op(in E p) }\n}\n";
  const auto document = xsmp::parse(world, "xsmpcat", text);
  const auto check = [&](std::string_view word, std::string_view declared) {
    SCOPED_TRACE(word);
    const auto at = static_cast<cursiv::TextOffset>(text.find(word));
    const auto position =
        cursiv::to_lsp(document->textDocument().positionAt(at));
    const auto links =
        world.language("xsmpcat").typeDefinitionProvider->getTypeDefinition(
            *document, position);
    ASSERT_TRUE(links);
    ASSERT_EQ(links->size(), 1u);
    const auto start = document->textDocument().offsetAt(
        cursiv::from_lsp(links->front().targetSelectionRange.start));
    EXPECT_EQ(start, text.find(declared));
  };
  check("f constant", "E {");
  check("k =", "E {");
  check("r def", "I {");
  check("p)", "E {");
  check("A = 0", "E {");
  check("E f", "E {");
}

TEST(XsmpDefinitionProvider, PlaceholdersAreReferencesOfTheirParameter) {
  auto world = xsmp::deploymentWorkspace();
  const std::string text =
      "assembly <Variant = \"Ops\"> A\nRoot: n.M\n{\n"
      "    kids += System{Variant}: n.M\n"
      "    kids += {Variant}: n.M\n}\n";
  const auto document =
      xsmp::parse(world, "xsmpasb", text, "file:///r/a.xsmpasb");
  const auto& language = world.language("xsmpasb");
  const auto at = static_cast<cursiv::TextOffset>(text.find("Variant"));
  const auto found = language.references->findDeclaration(*document, at);
  ASSERT_EQ(found.targets.size(), 1u);
  const auto first = text.find("{Variant}") + 1;
  const std::vector<std::size_t> expected{at, first,
                                          text.find("{Variant}", first) + 1};

  std::vector<std::size_t> rows;
  for (const auto& row : language.references->findReferences(
           *found.targets.front(), {.includeDeclaration = true})) {
    EXPECT_EQ(row.sourceEnd - row.sourceBegin, 7u);
    rows.push_back(row.sourceBegin);
  }
  EXPECT_EQ(rows, expected);

  const auto highlights =
      language.documentHighlightProvider->getDocumentHighlight(
          *document, cursiv::to_lsp(document->textDocument().positionAt(at)));
  EXPECT_EQ(highlights.size(), 3u);
}
