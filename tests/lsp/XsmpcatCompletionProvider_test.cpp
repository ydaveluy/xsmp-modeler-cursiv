#include <gtest/gtest.h>
#include <lsp/types.h>

#include <algorithm>
#include <regex>
#include <xsmp/Builtins.hpp>
#include <xsmp/Json.hpp>
#include <xsmp/utils/XsmpUtils.hpp>
#include <xsmp/validation/IssueCodes.hpp>

#include "support/LspTestSupport.hpp"

namespace {

const ::lsp::CompletionItem* findItem(const ::lsp::CompletionList& list,
                                      std::string_view label) {
  const auto item = std::ranges::find_if(
      list.items, [&](const auto& value) { return value.label == label; });
  return item == list.items.end() ? nullptr : &*item;
}

// The document once the item labelled `label` at the caret is accepted, its
// `@uuid` and tab stops resolved. Empty if no such item.
std::string accepted(xsmp::TestWorkspace& world, const std::string& text,
                     std::string_view label) {
  const std::string uri = "file:///accepted.xsmpcat";
  std::string out;
  (void)cursiv::test::expectCompletion(
      world.language("xsmpcat"),
      cursiv::test::ExpectedCompletionCallback{
          .text = text,
          .assertion =
              [&](const ::lsp::CompletionList& list) {
                const auto* item = findItem(list, label);
                if (!item || !item->textEdit) return;
                auto edits = item->additionalTextEdits.value_or(
                    std::vector<::lsp::TextEdit>{});
                edits.push_back(std::get<::lsp::TextEdit>(*item->textEdit));
                out = xsmp::applyTextEdits(
                    *world.shared().documents->getDocument(uri), edits);
              },
          .parseOptions = xsmp::documentAt(uri),
          .disposeAfterCheck = true});
  if (out.empty()) return {};
  out = std::regex_replace(
      out,
      std::regex(
          R"([0-9a-f]{8}-[0-9a-f]{4}-4[0-9a-f]{3}-[0-9a-f]{4}-[0-9a-f]{12})"),
      "U");
  return std::regex_replace(out, std::regex(R"(\$\{\d+:([^}]+)\}|\$\d+)"),
                            "$1");
}

}  // namespace

TEST(XsmpcatCompletionProvider, MultiplicitySnippetsProduceValidDeclarations) {
  auto ws = xsmp::xsmpcatWorkspace();
  ::lsp::CompletionList result;
  (void)cursiv::test::expectCompletion(
      ws.language("xsmpcat"),
      cursiv::test::ExpectedCompletionCallback{
          .text = "catalogue C\nnamespace n {\n model M {\n  <|>\n }\n}\n",
          .assertion = [&](const ::lsp::CompletionList& list) {
            result = list;
          }});
  for (const std::string_view label : {"Container", "Reference"}) {
    const auto item = std::ranges::find_if(
        result.items, [&](const auto& value) { return value.label == label; });
    ASSERT_NE(item, result.items.end());
    ASSERT_TRUE(item->insertText);
    auto concrete = std::regex_replace(
        *item->insertText, std::regex(R"(\$\{\d+\|([^,|}]+)[^}]*\|\})"), "$1");
    concrete =
        std::regex_replace(concrete, std::regex(R"(\$\{\d+:([^}]+)\})"), "$1");
    const auto document = xsmp::parse(
        ws, "xsmpcat",
        "catalogue Inserted namespace n { model M { " + concrete + " } }",
        "file:///inserted-" + std::string(label) + ".xsmpcat");
    EXPECT_TRUE(document->parseResult.diagnostics.empty()) << concrete;
  }
}

// Keyword completion at the entry-rule level (typing the "catalogue" keyword).
TEST(XsmpcatCompletionProvider, TopLevelKeywordCompletes) {
  auto ws = xsmp::xsmpcatWorkspace();
  (void)cursiv::test::expectCompletion(
      ws.language("xsmpcat"),
      cursiv::test::ExpectedCompletionCallback{
          .text = "cat<|>",
          .assertion = [](const ::lsp::CompletionList& completion) {
            EXPECT_TRUE(xsmp::hasLabel(completion, "Catalogue"));
          }});
}

// Keyword completion inside a nested rule body (a type declaration inside a
// namespace body) offers the type keywords although the declaration may start
// with optional attributes.
TEST(XsmpcatCompletionProvider, NamespaceBodyOffersTypeKeywords) {
  auto ws = xsmp::xsmpcatWorkspace();
  (void)cursiv::test::expectCompletion(
      ws.language("xsmpcat"),
      cursiv::test::ExpectedCompletionCallback{
          .text = "catalogue c\n"
                  "namespace n\n"
                  "{\n"
                  "  <|>\n"
                  "}\n",
          .assertion = [](const ::lsp::CompletionList& completion) {
            EXPECT_TRUE(xsmp::hasLabel(completion, "Structure"));
            EXPECT_TRUE(xsmp::hasLabel(completion, "Enumeration"));
          }});
}

// Cross-reference completion right after a bare `@`, before any type text is
// typed. The completion offers candidates from the parser's frontier even
// while the input is not yet a valid parse, so the attribute type stays
// mandatory in the grammar.
TEST(XsmpcatCompletionProvider, AttributeTypeCompletesRightAfterAt) {
  auto ws = xsmp::xsmpcatWorkspace();
  (void)cursiv::test::expectCompletion(
      ws.language("xsmpcat"),
      cursiv::test::ExpectedCompletionCallback{
          .text = "catalogue c\n"
                  "namespace n\n"
                  "{\n"
                  "  primitive Bool\n"
                  "  /** @usage PrimitiveType */\n"
                  "  attribute Bool Abstract\n"
                  "  @<|>\n"
                  "  primitive P\n"
                  "}\n",
          .assertion = [](const ::lsp::CompletionList& completion) {
            EXPECT_TRUE(xsmp::hasLabel(completion, "Abstract"));
          }});
}

// Cross-reference completion inside a nested body: a Field's type completes
// from the enclosing scope.
TEST(XsmpcatCompletionProvider, FieldTypeCompletesFromScope) {
  auto ws = xsmp::xsmpcatWorkspace();
  (void)cursiv::test::expectCompletion(
      ws.language("xsmpcat"),
      cursiv::test::ExpectedCompletionCallback{
          .text = "catalogue c\n"
                  "namespace n\n"
                  "{\n"
                  "  primitive Bool\n"
                  "  struct S { field Bo<|> }\n"
                  "}\n",
          .assertion = [](const ::lsp::CompletionList& completion) {
            EXPECT_TRUE(xsmp::hasLabel(completion, "Bool"));
          }});
}

TEST(XsmpcatCompletionProvider, TypeSnippetsContainFreshUuids) {
  auto ws = xsmp::xsmpcatWorkspace();
  (void)cursiv::test::expectCompletion(
      ws.language("xsmpcat"),
      cursiv::test::ExpectedCompletionCallback{
          .text = "catalogue Demo\nnamespace n\n{\n  <|>\n}\n",
          .assertion = [](const ::lsp::CompletionList& list) {
            const auto model = std::ranges::find_if(
                list.items,
                [](const auto& item) { return item.label == "Model"; });
            ASSERT_NE(model, list.items.end());
            ASSERT_TRUE(model->insertText);
            const auto& text = *model->insertText;
            EXPECT_TRUE(text.starts_with("/** @uuid "));
            EXPECT_TRUE(xsmp::isUUID(std::string_view(text).substr(10, 36)));
            EXPECT_NE(text.find("model ${1:Name}"), std::string::npos);
            EXPECT_FALSE(xsmp::hasLabel(list, "model"));
            EXPECT_FALSE(xsmp::hasLabel(list, "Field"));
          }});
}

TEST(XsmpcatCompletionProvider, CatalogueScopeOffersNamespaceOnly) {
  auto ws = xsmp::xsmpcatWorkspace();
  (void)cursiv::test::expectCompletion(
      ws.language("xsmpcat"),
      cursiv::test::ExpectedCompletionCallback{
          .text = "catalogue Demo\nnamespace n { model M {} }\n<|>",
          .assertion = [](const ::lsp::CompletionList& list) {
            EXPECT_TRUE(xsmp::hasLabel(list, "Namespace"));
            EXPECT_FALSE(xsmp::hasLabel(list, "Model"));
            EXPECT_FALSE(xsmp::hasLabel(list, "Field"));
          }});
}

TEST(XsmpcatCompletionProvider, ModelScopeOffersMembers) {
  auto ws = xsmp::xsmpcatWorkspace();
  (void)cursiv::test::expectCompletion(
      ws.language("xsmpcat"),
      cursiv::test::ExpectedCompletionCallback{
          .text = "catalogue Demo\nnamespace n { model M\n{\n <|>\n} }\n",
          .assertion = [](const ::lsp::CompletionList& list) {
            EXPECT_TRUE(xsmp::hasLabel(list, "Field"));
            EXPECT_TRUE(xsmp::hasLabel(list, "Reference"));
            EXPECT_FALSE(xsmp::hasLabel(list, "Model"));
            EXPECT_FALSE(xsmp::hasLabel(list, "field"));
          }});
}

TEST(XsmpcatCompletionProvider, TypedBooleansPrecedeMatchingConstants) {
  auto ws = xsmp::xsmpcatWorkspace();
  (void)cursiv::test::expectCompletion(
      ws.language("xsmpcat"),
      cursiv::test::ExpectedCompletionCallback{
          .text =
              "catalogue Demo\nnamespace Smp { public primitive Bool public "
              "primitive Int32 }\nnamespace n { model M\n{\n public constant "
              "Smp.Bool Enabled = true\n public constant Smp.Int32 Count = "
              "0\n field Smp.Bool enabled =<|>\n} }\n",
          .assertion = [](const ::lsp::CompletionList& list) {
            ASSERT_GE(list.items.size(), 2);
            EXPECT_EQ(list.items[0].label, "false");
            EXPECT_EQ(list.items[1].label, "true");
            EXPECT_TRUE(xsmp::hasLabel(list, "Enabled"));
            EXPECT_FALSE(xsmp::hasLabel(list, "Count"));
            EXPECT_FALSE(xsmp::hasLabel(list, "nullptr"));
            EXPECT_TRUE(list.items[0].preselect.value_or(false));
            ASSERT_TRUE(list.items[0].textEdit);
            EXPECT_EQ(
                std::get<::lsp::TextEdit>(*list.items[0].textEdit).newText,
                " false");
          }});
}

TEST(XsmpcatCompletionProvider, TypedEnumAndArrayDefaults) {
  auto ws = xsmp::xsmpcatWorkspace();
  (void)cursiv::test::expectCompletion(
      ws.language("xsmpcat"),
      cursiv::test::ExpectedCompletionCallback{
          .text = "catalogue Demo\nnamespace n { enum Mode { Off = 0, On = 1 } "
                  "model M\n{\n field Mode mode =<|>\n} }\n",
          .assertion = [](const ::lsp::CompletionList& list) {
            EXPECT_TRUE(xsmp::hasLabel(list, "n.Mode.Off"));
            EXPECT_TRUE(xsmp::hasLabel(list, "n.Mode.On"));
            EXPECT_FALSE(xsmp::hasLabel(list, "nullptr"));
          }});
  (void)cursiv::test::expectCompletion(
      ws.language("xsmpcat"),
      cursiv::test::ExpectedCompletionCallback{
          .text =
              "catalogue Other\nnamespace Smp { primitive Bool }\nnamespace n "
              "{ array Bools = Smp.Bool[2] model M\n{\n field Bools values "
              "=<|>\n} }\n",
          .assertion = [](const ::lsp::CompletionList& list) {
            ASSERT_FALSE(list.items.empty());
            EXPECT_EQ(list.items[0].label, "{false, false}");
          }});
}

TEST(XsmpcatCompletionProvider, FieldCandidatesDependOnOwner) {
  auto ws = xsmp::xsmpcatWorkspace();
  (void)cursiv::test::expectCompletion(
      ws.language("xsmpcat"),
      cursiv::test::ExpectedCompletionCallback{
          .text =
              "catalogue Demo\nnamespace n { public interface IBus {} public "
              "model Child {} public enum State { Ready = 0 } model M\n{\n "
              "field <|> value\n} }\n",
          .assertion = [](const ::lsp::CompletionList& list) {
            EXPECT_TRUE(xsmp::hasLabel(list, "State"));
            EXPECT_FALSE(xsmp::hasLabel(list, "IBus"));
            EXPECT_FALSE(xsmp::hasLabel(list, "Child"));
          }});
}

TEST(XsmpcatCompletionProvider, ValueOnTheLineAfterTheEquals) {
  auto world = xsmp::xsmpcatWorkspace();
  (void)cursiv::test::expectCompletion(
      world.language("xsmpcat"),
      cursiv::test::ExpectedCompletionCallback{
          .text =
              "catalogue c\nnamespace Smp { public primitive Bool } namespace "
              "n { public struct K { constant Smp.Bool flag =\n   <|> } }",
          .assertion = [](const ::lsp::CompletionList& list) {
            EXPECT_TRUE(xsmp::hasLabel(list, "true"));
            EXPECT_TRUE(xsmp::hasLabel(list, "false"));
          }});
}

TEST(XsmpcatCompletionProvider, TypeReplacesTheWholeNameAtTheCaret) {
  auto world = xsmp::xsmpcatWorkspace();
  for (const std::string name : {"Bool", "Smp.Bool"})
    (void)cursiv::test::expectCompletion(
        world.language("xsmpcat"),
        cursiv::test::ExpectedCompletionCallback{
            .text = "catalogue c\nnamespace n { public struct Other {} model M "
                    "{\nfield <|>"
                    + name + " x } }",
            .assertion = [&](const ::lsp::CompletionList& list) {
              const auto item =
                  std::ranges::find_if(list.items, [](const auto& candidate) {
                    return candidate.label == "Other";
                  });
              ASSERT_NE(item, list.items.end());
              ASSERT_TRUE(item->textEdit);
              const auto& edit = std::get<::lsp::TextEdit>(*item->textEdit);
              EXPECT_EQ(edit.range.start.character, 6u);
              EXPECT_EQ(edit.range.end.character, 6u + name.size());
            }});
}

TEST(XsmpcatCompletionProvider,
     DesignatedInitializerOffersTheFieldsOfItsStructure) {
  auto world = xsmp::xsmpcatWorkspace();
  for (const std::string before : {"", ".x = 1, "})
    (void)cursiv::test::expectCompletion(
        world.language("xsmpcat"),
        cursiv::test::ExpectedCompletionCallback{
            .text =
                "catalogue c\nnamespace n { public struct Point { public "
                "field Smp.Int32 x public field Smp.Int32 y } model M { field "
                "Point p = {"
                + before + ".<|>} } }",
            .assertion = [](const ::lsp::CompletionList& list) {
              EXPECT_TRUE(xsmp::hasLabel(list, "y"));
              EXPECT_FALSE(xsmp::hasLabel(list, "p"));
              EXPECT_FALSE(xsmp::hasLabel(list, "Point"));
            }});
}

TEST(XsmpcatCompletionProvider,
     AttributeCandidatesRespectUsageAndMultiplicity) {
  auto world = xsmp::xsmpcatWorkspace();
  (void)cursiv::test::expectCompletion(
      world.language("xsmpcat"),
      cursiv::test::ExpectedCompletionCallback{
          .text =
              "catalogue c\nnamespace n { primitive Bool /** @usage Model */ "
              "public attribute Bool ModelOnly /** @usage Field */ public "
              "attribute Bool FieldOnly public attribute Bool Unusable @<|> "
              "model M {} }",
          .assertion = [](const auto& list) {
            EXPECT_TRUE(xsmp::hasLabel(list, "ModelOnly"));
            EXPECT_FALSE(xsmp::hasLabel(list, "FieldOnly"));
            EXPECT_FALSE(xsmp::hasLabel(list, "Unusable"));
          }});
}

TEST(XsmpcatCompletionProvider, ValueReferencesMatchExpectedType) {
  auto world = xsmp::xsmpcatWorkspace();
  (void)cursiv::test::expectCompletion(
      world.language("xsmpcat"),
      cursiv::test::ExpectedCompletionCallback{
          .text =
              "catalogue c\nnamespace Smp { public primitive Bool public "
              "primitive Int32 } namespace n { public struct Values { public "
              "constant Smp.Bool Yes = "
              "true public constant Smp.Int32 Count = 1 field "
              "Smp.Bool flag = Ye<|> } }",
          .assertion = [](const auto& list) {
            EXPECT_TRUE(xsmp::hasLabel(list, "Yes"));
            EXPECT_FALSE(xsmp::hasLabel(list, "Count"));
          }});
}

TEST(XsmpcatCompletionProvider,
     ReferenceCandidatesCarryDocumentationAndDeprecatedTag) {
  auto world = xsmp::xsmpcatWorkspace();
  (void)cursiv::test::expectCompletion(
      world.language("xsmpcat"),
      cursiv::test::ExpectedCompletionCallback{
          .text = "catalogue c\nnamespace n { /** Use the replacement.\n * "
                  "@deprecated Prefer New.\n */ public struct Old {} model M { "
                  "field Ol<|> value } }",
          .assertion = [](const auto& list) {
            const auto item = std::ranges::find_if(
                list.items,
                [](const auto& item) { return item.label == "Old"; });
            ASSERT_NE(item, list.items.end());
            EXPECT_EQ(item->detail, "Field Definition");
            EXPECT_EQ(item->sortText, "1000_0001_Old");
            EXPECT_TRUE(item->documentation);
            ASSERT_TRUE(item->tags);
            EXPECT_FALSE(item->tags->empty());
          }});
}

TEST(XsmpcatCompletionProvider, AssertionContracts) {
  using namespace xsmp;
  const auto fixture =
      ::lsp::json::parse(readText(std::filesystem::path(XSMP_FIXTURES_DIR)
                                  / "lsp/catalogue-completion-contracts.json"));
  ASSERT_EQ(fixture.object().get("cases").array().size(), 24U);
  for (const auto& entry : fixture.object().get("cases").array()) {
    const auto& record = entry.object();
    SCOPED_TRACE(stringAt(record, "name"));
    auto world = xsmp::xsmpcatWorkspace();
    const auto uri = stringAt(record, "uri");
    ::lsp::CompletionList list;
    std::shared_ptr<cursiv::Document> document;
    (void)cursiv::test::expectCompletion(
        world.language("xsmpcat"),
        cursiv::test::ExpectedCompletionCallback{
            .text = stringAt(record, "text"),
            .assertion =
                [&](const ::lsp::CompletionList& answer) {
                  list = answer;
                  document = world.shared().documents->getDocument(uri);
                },
            .parseOptions = xsmp::documentAt(uri)});
    ASSERT_TRUE(document);
    const auto find = [&](std::string_view label) {
      return std::ranges::find_if(
          list.items, [&](const auto& item) { return item.label == label; });
    };
    const auto insertion = [](const ::lsp::CompletionItem& item) {
      if (item.textEdit)
        return std::visit([](const auto& edit) { return edit.newText; },
                          *item.textEdit);
      return item.insertText.value_or("");
    };
    for (const auto& raw : record.get("assertions").array()) {
      const auto& assertion = raw.object();
      const auto kind = stringAt(assertion, "kind"),
                 label = stringAt(assertion, "label");
      SCOPED_TRACE(kind + ": " + label);
      const auto item = find(label);
      if (kind == "present")
        EXPECT_NE(item, list.items.end());
      else if (kind == "absent")
        EXPECT_EQ(item, list.items.end());
      else if (kind == "count") {
        EXPECT_EQ(std::ranges::count_if(list.items,
                                        [&](const auto& candidate) {
                                          return candidate.label == label;
                                        }),
                  assertion.get("value").integer());
      } else if (kind == "before") {
        const auto second = find(stringAt(assertion, "value"));
        EXPECT_NE(item, list.items.end());
        EXPECT_NE(second, list.items.end());
        EXPECT_LT(item - list.items.begin(), second - list.items.begin());
      } else if (kind == "labelAt" || kind == "preselectAt"
                 || kind == "insertStartsWith") {
        const auto index =
            static_cast<std::size_t>(assertion.get("index").integer());
        ASSERT_LT(index, list.items.size());
        const auto& candidate = list.items[index];
        if (kind == "labelAt")
          EXPECT_EQ(candidate.label, label);
        else if (kind == "preselectAt")
          EXPECT_EQ(candidate.preselect, assertion.get("value").boolean());
        else
          EXPECT_TRUE(
              insertion(candidate).starts_with(stringAt(assertion, "value")));
      } else {
        ASSERT_NE(item, list.items.end());
        const auto expected = stringAt(assertion, "value");
        if (kind == "editEquals")
          EXPECT_EQ(insertion(*item), expected);
        else if (kind == "insertContains")
          EXPECT_NE(item->insertText.value_or("").find(expected),
                    std::string::npos);
        else if (kind == "applyContains") {
          ASSERT_TRUE(item->textEdit);
          const auto text = std::visit(
              [&](const auto& edit) {
                ::lsp::TextEdit value;
                if constexpr (requires { edit.range; })
                  value.range = edit.range;
                else
                  value.range = edit.replace;
                value.newText = edit.newText;
                return xsmp::applyTextEdits(*document, {value});
              },
              *item->textEdit);
          EXPECT_NE(text.find(expected), std::string::npos);
        } else
          FAIL() << "Unknown completion assertion: " << kind;
      }
    }
  }
}

TEST(XsmpcatCompletionProvider, KeywordsCompleteAfterTheStartOfADeclaration) {
  auto world = xsmp::xsmpcatWorkspace();
  const auto check = [&](const std::string& body, std::string_view label) {
    SCOPED_TRACE(body);
    (void)cursiv::test::expectCompletion(
        world.language("xsmpcat"),
        cursiv::test::ExpectedCompletionCallback{
            .text = "catalogue c\nnamespace n {\n" + body + "\n}\n",
            .assertion = [&](const ::lsp::CompletionList& list) {
              EXPECT_TRUE(xsmp::hasLabel(list, label));
            }});
  };
  check("  public cl<|>", "Class");
  check("  abstract cl<|>", "Class");
  check("  @Foo cl<|>", "Class");
  check("  @Foo(1) public <|>", "Model");
  check("  /** doc */ cl<|>", "Class");
  check("  struct S { private fi<|> }", "Field");
  check("  model M { field Smp.Bool a\n private <|> }", "Field");
}

TEST(XsmpcatCompletionProvider, ModifiersAdmitOnlyWhatTheyApplyTo) {
  auto world = xsmp::xsmpcatWorkspace();
  (void)cursiv::test::expectCompletion(
      world.language("xsmpcat"),
      cursiv::test::ExpectedCompletionCallback{
          .text = "catalogue c\nnamespace n {\n abstract <|>\n}\n",
          .assertion = [](const ::lsp::CompletionList& list) {
            EXPECT_TRUE(xsmp::hasLabel(list, "Class"));
            EXPECT_TRUE(xsmp::hasLabel(list, "Model"));
            EXPECT_FALSE(xsmp::hasLabel(list, "Structure"));
            EXPECT_FALSE(xsmp::hasLabel(list, "Enumeration"));
          }});
}

TEST(XsmpcatCompletionProvider, UuidGoesBeforeWhatTheDeclarationStartsWith) {
  auto world = xsmp::xsmpcatWorkspace();
  const std::string head = "catalogue c\nnamespace n {\n";
  const auto check = [&](const std::string& before, const std::string& after) {
    SCOPED_TRACE(before);
    const auto out = accepted(world, head + before + "\n}\n", "Primitive");
    EXPECT_EQ(out, head + after + "\n}\n");
    const auto result =
        cursiv::test::validationHelper(world.language("xsmpcat"), out);
    EXPECT_FALSE(std::ranges::any_of(result.diagnostics,
                                     [](const auto& diagnostic) {
                                       return cursiv::data_code(diagnostic)
                                              == xsmp::MissingUuid;
                                     }))
        << out;
  };
  check("  <|>", "  /** @uuid U */\nprimitive Name");
  check("  public <|>", "  /** @uuid U */\n  public primitive Name");
  check("  public pri<|>", "  /** @uuid U */\n  public primitive Name");
  check("  @Foo(1) public <|>",
        "  /** @uuid U */\n  @Foo(1) public primitive Name");
  check("  /** a */\n  public <|>",
        "  /** a\n   * @uuid U\n   */\n  public primitive Name");
  check("  /** a */\n  <|>", "  /** a\n   * @uuid U\n   */\n  primitive Name");
  check("  /** @uuid x */\n  public <|>",
        "  /** @uuid x */\n  public primitive Name");
  // A whole declaration before the modifiers stays out of it.
  EXPECT_EQ(
      accepted(world, head + "  enum E {}\n  public <|>\n}\n", "Primitive"),
      head + "  enum E {}\n  /** @uuid U */\n  public primitive Name\n}\n");
}

TEST(XsmpcatCompletionProvider, NamePositionsOfferNoKeyword) {
  auto world = xsmp::xsmpcatWorkspace();
  for (const std::string text :
       {"catalogue <|>", "catalogue c\nnamespace <|>",
        "catalogue c\nnamespace n { struct <|> }",
        "catalogue c\nnamespace n { using <|> }",
        "catalogue c\nnamespace n { struct S { field n.S <|> } }"})
    (void)cursiv::test::expectCompletion(
        world.language("xsmpcat"),
        cursiv::test::ExpectedCompletionCallback{
            .text = text, .assertion = [&](const auto& list) {
              EXPECT_TRUE(list.items.empty()) << text;
            }});
  // `in` is still the direction of a parameter.
  (void)cursiv::test::expectCompletion(
      world.language("xsmpcat"),
      cursiv::test::ExpectedCompletionCallback{
          .text = "catalogue c\nnamespace n { model M { def void f(<|>) } }",
          .assertion = [](const auto& list) {
            EXPECT_TRUE(xsmp::hasLabel(list, "in"));
            EXPECT_TRUE(xsmp::hasLabel(list, "inout"));
          }});
}

TEST(XsmpcatCompletionProvider, EnumerationBodyOffersTheLiteralSnippet) {
  auto world = xsmp::xsmpcatWorkspace();
  for (const std::string body : {"<|>", "A = 1, <|>"})
    (void)cursiv::test::expectCompletion(
        world.language("xsmpcat"),
        cursiv::test::ExpectedCompletionCallback{
            .text = "catalogue c\nnamespace n { enum E { " + body + " } }",
            .assertion = [](const ::lsp::CompletionList& list) {
              ASSERT_EQ(list.items.size(), 1u);
              EXPECT_EQ(list.items[0].label, "Enumeration Literal");
              EXPECT_EQ(list.items[0].insertText, "${1:Literal} = ${2:0}");
            }});
  // Not for the name of the enumeration, nor after a whole literal.
  for (const std::string text :
       {"enum <|>", "enum E { A <|> }", "enum E { A = 1 <|> }"})
    (void)cursiv::test::expectCompletion(
        world.language("xsmpcat"),
        cursiv::test::ExpectedCompletionCallback{
            .text = "catalogue c\nnamespace n { " + text + " }",
            .assertion = [](const ::lsp::CompletionList& list) {
              EXPECT_FALSE(xsmp::hasLabel(list, "Enumeration Literal"));
            }});
}

TEST(XsmpcatCompletionProvider, TypeAddsOnlyWhatIsNotWrittenYet) {
  auto world = xsmp::xsmpcatWorkspace();
  const std::string head =
      "catalogue c\nnamespace n { public primitive P public interface I {} "
      "public model C {} ";
  const auto check = [&](const std::string& body, std::string_view label,
                         const std::string& expected) {
    SCOPED_TRACE(body);
    EXPECT_EQ(accepted(world, head + body + " }", label),
              head + expected + " }");
  };
  check("using U = <|>", "P", "using U = P*");
  check("using U = <|>*", "P", "using U = P*");
  check("using U = <|>Q*", "P", "using U = P*");
  check("array A = <|>", "P", "array A = P[1]");
  check("array A = <|>[3]", "P", "array A = P[3]");
  check("array A = P<|>[3]", "P", "array A = P[3]");
  check("model M { reference <|> r }", "I", "model M { reference I* r }");
  check("model M { reference I<|>* r }", "I", "model M { reference I* r }");
  check("model M { reference I<|>? r }", "I", "model M { reference I? r }");
  check("model M { reference I<|>[2] r }", "I", "model M { reference I[2] r }");
  check("model M { container <|> r }", "C", "model M { container C* r }");
  check("model M { container C<|>+ r }", "C", "model M { container C+ r }");
}

TEST(XsmpcatCompletionProvider, ValueAfterTheEqualsReadsApart) {
  auto world = xsmp::xsmpcatWorkspace();
  xsmp::loadXsmpcatBuiltins(world.shared());
  const std::string head = "catalogue c\nnamespace n { model M { ";
  EXPECT_EQ(accepted(world, head + "field Smp.Int32 x =<|> } }", "0"),
            head + "field Smp.Int32 x = 0 } }");
  EXPECT_EQ(accepted(world, head + "field Smp.Int32 x = <|> } }", "0"),
            head + "field Smp.Int32 x = 0 } }");
  EXPECT_EQ(accepted(world, head + "field Smp.Int32 x =\n   <|> } }", "0"),
            head + "field Smp.Int32 x =\n   0 } }");
}

TEST(XsmpcatCompletionProvider, NoValueInsideAToken) {
  auto world = xsmp::xsmpcatWorkspace();
  xsmp::loadXsmpcatBuiltins(world.shared());
  for (const std::string value : {"\"ab<|>c\"", "12<|>3"})
    (void)cursiv::test::expectCompletion(
        world.language("xsmpcat"),
        cursiv::test::ExpectedCompletionCallback{
            .text = "catalogue c\nnamespace n { model M { field Smp.Int32 x = "
                    + value + " } }",
            .assertion = [](const ::lsp::CompletionList& list) {
              EXPECT_FALSE(xsmp::hasLabel(list, "0"));
            }});
}

TEST(XsmpcatCompletionProvider, InitializerOfAMemberIsNotTheValueOfTheField) {
  auto world = xsmp::xsmpcatWorkspace();
  xsmp::loadXsmpcatBuiltins(world.shared());
  (void)cursiv::test::expectCompletion(
      world.language("xsmpcat"),
      cursiv::test::ExpectedCompletionCallback{
          .text = "catalogue c\nnamespace n { public struct P { public field "
                  "Smp.Int32 x } model M { field P p = { .x = <|> } } }",
          .assertion = [](const ::lsp::CompletionList& list) {
            EXPECT_FALSE(xsmp::hasLabel(list, "{.x = 0}"));
          }});
}

TEST(XsmpcatCompletionProvider, DefaultValuesStayShort) {
  auto world = xsmp::xsmpcatWorkspace();
  xsmp::loadXsmpcatBuiltins(world.shared());
  const std::string head = "catalogue c\nnamespace n {\n";
  // A structure that holds itself, several times.
  (void)cursiv::test::expectCompletion(
      world.language("xsmpcat"),
      cursiv::test::ExpectedCompletionCallback{
          .text = head + "struct T { field T a field T b field T c = <|> }\n}",
          .assertion = [](const ::lsp::CompletionList& list) {
            ASSERT_FALSE(list.items.empty());
            EXPECT_LT(list.items[0].label.size(), 200u) << list.items[0].label;
          }});
  // Arrays inside arrays.
  (void)cursiv::test::expectCompletion(
      world.language("xsmpcat"),
      cursiv::test::ExpectedCompletionCallback{
          .text = head
                  + "array A = Smp.Int32[1000] array B = A[1000] array C = "
                    "B[1000] struct S { field C x = <|> }\n}",
          .assertion = [](const ::lsp::CompletionList& list) {
            ASSERT_FALSE(list.items.empty());
            EXPECT_LE(list.items[0].label.size(), 4096u);
          }});
  // A small array and a small structure are written out.
  (void)cursiv::test::expectCompletion(
      world.language("xsmpcat"),
      cursiv::test::ExpectedCompletionCallback{
          .text = head
                  + "array A = Smp.Int32[2] public struct P { public field A a "
                    "public field Smp.Bool b } struct S { field P x = <|> }\n}",
          .assertion = [](const ::lsp::CompletionList& list) {
            ASSERT_FALSE(list.items.empty());
            EXPECT_EQ(list.items[0].label, "{.a = {0, 0}, .b = false}");
          }});
}

TEST(XsmpcatCompletionProvider, TriggerCharacters) {
  auto world = xsmp::xsmpcatWorkspace();
  EXPECT_EQ(world.language("xsmpcat")
                .completionProvider->completionOptions()
                .triggerCharacters,
            (std::vector<std::string>{"=", "@"}));
}

TEST(XsmpcatCompletionProvider, OffersAnAttributeOnAnArrayThroughAParentUsage) {
  auto world = xsmp::xsmpcatWorkspace();
  xsmp::loadXsmpcatBuiltins(world.shared<xsmp::XsmpSharedServices>());
  (void)cursiv::test::expectCompletion(
      world.language("xsmpcat"),
      cursiv::test::ExpectedCompletionCallback{
          .text = R"(catalogue demo
namespace demo
{
    public primitive Bool
    public primitive Int32
    /** @usage ValueType */
    public attribute Bool OnValueType = true
    /** @usage Model */
    public attribute Bool OnModel = true
    @<|>
    public array Values = demo.Int32[4]
}
)",
          .assertion = [](const ::lsp::CompletionList& list) {
            EXPECT_NE(findItem(list, "OnValueType"), nullptr);
            EXPECT_EQ(findItem(list, "OnModel"), nullptr);
          }});
}
