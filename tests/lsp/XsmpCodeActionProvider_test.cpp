#include <cursiv/actions/CodeActionProvider.hpp>
#include <cursiv/parser/Parser.hpp>
#include <cursiv/validation/DocumentValidator.hpp>
#include <xsmp/validation/IssueCodes.hpp>

#include "support/LspTestSupport.hpp"

namespace {

xsmp::TestWorkspace makeWorkspace() {
  auto world = xsmp::TestWorkspace::withShared<xsmp::XsmpSharedServices>();
  auto& shared = world.shared();
  shared.projectManager = std::make_unique<xsmp::ProjectManager>(shared);
  world.registerLanguage(xsmp::createXsmpcatServices(shared));
  xsmp::registerXsmpasbServices(shared);
  xsmp::registerXsmpcfgServices(shared);
  xsmp::registerXsmplnkServices(shared);
  xsmp::registerXsmpsedServices(shared);
  return world;
}

}  // namespace

TEST(XsmpCodeActionProvider, MissingUuidFixPreservesExistingDescription) {
  auto world = makeWorkspace();
  const auto result = cursiv::test::validationHelper(
      world.language("xsmpcat"),
      "catalogue c\nnamespace n { /** A documented "
      "structure. */ struct S {} }\n");
  ::lsp::CodeActionParams params;
  params.textDocument.uri = ::lsp::DocumentUri::parse(result.document->uri);
  for (const auto& diagnostic : result.diagnostics)
    if (cursiv::data_code(diagnostic) == xsmp::MissingUuid) {
      ::lsp::Diagnostic wire;
      wire.code = std::string(xsmp::MissingUuid);
      wire.range = cursiv::to_lsp(result.document->textDocument().rangeAt(
          diagnostic.range.begin, diagnostic.range.end));
      wire.message = diagnostic.message;
      params.context.diagnostics.push_back(std::move(wire));
    }
  const auto actions =
      world.language("xsmpcat").codeActionProvider->getCodeActions(
          *result.document, params);
  const auto fix = std::ranges::find_if(actions, [](const auto& action) {
    return action.title == "Generate missing UUID.";
  });
  ASSERT_NE(fix, actions.end());
  ASSERT_TRUE(fix->edit && fix->edit->changes);
  const auto changed = xsmp::applyTextEdits(
      *result.document, fix->edit->changes->begin()->second);
  EXPECT_NE(changed.find("A documented structure."), std::string::npos);
  EXPECT_NE(changed.find("@uuid "), std::string::npos);
  const auto fixed =
      cursiv::test::validationHelper(world.language("xsmpcat"), changed);
  EXPECT_TRUE(fixed.document->parseSucceeded());
  EXPECT_FALSE(
      std::ranges::any_of(fixed.diagnostics, [](const auto& diagnostic) {
        return cursiv::data_code(diagnostic) == xsmp::MissingUuid;
      }));
}

namespace {

// What the `Generate missing UUID.` action makes of `text`.
std::string withMissingUuid(xsmp::TestWorkspace& world,
                            const std::string& text) {
  const auto result =
      cursiv::test::validationHelper(world.language("xsmpcat"), text);
  ::lsp::CodeActionParams params;
  params.textDocument.uri = ::lsp::DocumentUri::parse(result.document->uri);
  for (const auto& diagnostic : result.diagnostics)
    if (cursiv::data_code(diagnostic) == xsmp::MissingUuid) {
      ::lsp::Diagnostic wire;
      wire.code = std::string(xsmp::MissingUuid);
      wire.range = cursiv::to_lsp(result.document->textDocument().rangeAt(
          diagnostic.range.begin, diagnostic.range.end));
      params.context.diagnostics.push_back(std::move(wire));
    }
  const auto actions =
      world.language("xsmpcat").codeActionProvider->getCodeActions(
          *result.document, params);
  if (actions.empty()) return "<no action>";
  auto changed = xsmp::applyTextEdits(
      *result.document, actions.front().edit->changes->begin()->second);
  // The UUID is new each time.
  const auto tag = changed.find("@uuid ");
  return tag == std::string::npos ? changed : changed.replace(tag + 6, 36, "U");
}

}  // namespace

TEST(XsmpCodeActionProvider, MissingUuidTakesItsOwnLineInTheDocComment) {
  auto world = makeWorkspace();
  const std::string head = "catalogue c\nnamespace n\n{\n";
  const auto check = [&](std::string_view before, std::string_view after) {
    EXPECT_EQ(withMissingUuid(world, head + std::string(before) + "\n}\n"),
              head + std::string(after) + "\n}\n");
  };
  // No comment, then a comment with its `*/` after some text on one line, then
  // on several, then on a line of its own.
  check("  struct S {}", "  /** @uuid U */\n  struct S {}");
  check("\tstruct S {}", "\t/** @uuid U */\n\tstruct S {}");
  check("  /** a */\n  struct S {}",
        "  /** a\n   * @uuid U\n   */\n  struct S {}");
  check("  /** a\n      b */\n  struct S {}",
        "  /** a\n      b\n   * @uuid U\n   */\n  struct S {}");
  check("  /**\n   * a\n   */\n  struct S {}",
        "  /**\n   * a\n   * @uuid U\n   */\n  struct S {}");
  // Before the attributes and the modifiers.
  check("  public struct S {}", "  /** @uuid U */\n  public struct S {}");
}

TEST(XsmpCodeActionProvider, SyntaxErrorInAPathOffersNoUnsafe) {
  auto world = makeWorkspace();
  const std::string text = "configuration Demo\n/Root { value = 1i32 }\n";
  const auto document = xsmp::parse(world, "xsmpcfg", text);
  ::lsp::CodeActionParams params;
  params.textDocument.uri = ::lsp::DocumentUri::parse(document->uri);
  for (const auto code : {cursiv::kParseRepaired, cursiv::kSyntaxError,
                          cursiv::kConversionError}) {
    ::lsp::Diagnostic wire;
    wire.message = "Unexpected token";
    wire.data = ::lsp::LSPObject{{"code", std::string(code)}};
    wire.range = cursiv::to_lsp(document->textDocument().rangeAt(
        static_cast<cursiv::TextOffset>(text.find("Root")),
        static_cast<cursiv::TextOffset>(text.find("Root") + 4)));
    params.context.diagnostics.push_back(std::move(wire));
  }
  EXPECT_TRUE(world.language("xsmpcfg")
                  .codeActionProvider->getCodeActions(*document, params)
                  .empty());
}

TEST(XsmpCodeActionProvider, InvalidDeploymentPathCanBeMarkedUnsafe) {
  auto world = makeWorkspace();
  const std::string text = "configuration Demo\n/Root { value = 1i32 }\n";
  const auto document = xsmp::parse(world, "xsmpcfg", text);
  ::lsp::CodeActionParams params;
  params.textDocument.uri = ::lsp::DocumentUri::parse(document->uri);
  ::lsp::Diagnostic wire;
  wire.message = "Unknown root";
  wire.range = cursiv::to_lsp(document->textDocument().rangeAt(
      static_cast<cursiv::TextOffset>(text.find("Root")),
      static_cast<cursiv::TextOffset>(text.find("Root") + 4)));
  params.context.diagnostics.push_back(std::move(wire));
  auto second = params.context.diagnostics.front();
  second.message = "Another issue on the same path";
  params.context.diagnostics.push_back(std::move(second));
  const auto actions =
      world.language("xsmpcfg").codeActionProvider->getCodeActions(*document,
                                                                   params);
  const auto fix = std::ranges::find_if(actions, [](const auto& action) {
    return action.title == "Declare as `unsafe`.";
  });
  ASSERT_NE(fix, actions.end());
  ASSERT_TRUE(fix->diagnostics);
  EXPECT_EQ(fix->diagnostics->size(), 2U);
  ASSERT_TRUE(fix->edit && fix->edit->changes);
  const auto changed =
      xsmp::applyTextEdits(*document, fix->edit->changes->begin()->second);
  EXPECT_NE(changed.find("unsafe /Root"), std::string::npos);
  EXPECT_TRUE(world.language("xsmpcfg").parser->parse(changed).accepted);
}
