#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <string_view>
#include <xsmp/Contributions.hpp>
#include <xsmp/Json.hpp>

#include "support/LspTestSupport.hpp"

// --- Completion (cursiv default provider) ----------------------------------

// Top-level keyword completion — the entry rule's `Project | Profile | Tool`
// choice offers the `project` keyword.
TEST(XsmpprojectCompletionProvider, TopLevelKeyword) {
  auto ws = xsmp::xsmpprojectWorkspace();
  (void)cursiv::test::expectCompletion(
      ws.language("xsmpproject"),
      cursiv::test::ExpectedCompletionCallback{
          .text = "pro<|>",
          .assertion =
              [](const ::lsp::CompletionList& completion) {
                EXPECT_TRUE(xsmp::hasLabel(completion, "Project"));
              },
          .parseOptions = xsmp::documentAt("file:///test/0/xsmp.project"),
          .disposeAfterCheck = true});
}

// Element keyword completion inside the project body offers `dependency`.
TEST(XsmpprojectCompletionProvider, ProjectBodyOffersElementKeywords) {
  auto ws = xsmp::xsmpprojectWorkspace();
  (void)cursiv::test::expectCompletion(
      ws.language("xsmpproject"),
      cursiv::test::ExpectedCompletionCallback{
          .text = "project 'P'\nsource 'src'\n<|>",
          .assertion =
              [](const ::lsp::CompletionList& completion) {
                EXPECT_TRUE(
                    xsmp::hasLabel(completion, "dependency '<project-name>'"));
              },
          .parseOptions = xsmp::documentAt("file:///test/1/xsmp.project"),
          .disposeAfterCheck = true});
}

// Root keyword completion is filtered by the file kind: an `xsmp.project` file
// offers `project` but not `profile`/`tool`.
TEST(XsmpprojectCompletionProvider, RootKeywordFilteredForProjectFile) {
  auto ws = xsmp::xsmpprojectWorkspace();
  (void)cursiv::test::expectCompletion(
      ws.language("xsmpproject"),
      cursiv::test::ExpectedCompletionCallback{
          .text = "<|>",
          .assertion =
              [](const ::lsp::CompletionList& completion) {
                EXPECT_TRUE(xsmp::hasLabel(completion, "Project"));
                EXPECT_FALSE(xsmp::hasLabel(completion, "profile"));
                EXPECT_FALSE(xsmp::hasLabel(completion, "tool"));
              },
          .parseOptions = xsmp::documentAt("file:///p/xsmp.project"),
          .disposeAfterCheck = true});
}

// … and a `.xsmpprofile` file offers `profile` but not `project`/`tool`.
TEST(XsmpprojectCompletionProvider, RootKeywordFilteredForProfileFile) {
  auto ws = xsmp::xsmpprojectWorkspace();
  (void)cursiv::test::expectCompletion(
      ws.language("xsmpproject"),
      cursiv::test::ExpectedCompletionCallback{
          .text = "<|>",
          .assertion =
              [](const ::lsp::CompletionList& completion) {
                EXPECT_TRUE(xsmp::hasLabel(completion, "Profile"));
                EXPECT_FALSE(xsmp::hasLabel(completion, "project"));
                EXPECT_FALSE(xsmp::hasLabel(completion, "tool"));
              },
          .parseOptions = xsmp::documentAt("file:///p/sdk.xsmpprofile"),
          .disposeAfterCheck = true});
}

// Reference completion inserts the quoted name (the grammar's STRING terminal),
// not a bare identifier that would fail to parse.
TEST(XsmpprojectCompletionProvider, ReferenceCandidatesAreQuoted) {
  auto ws = xsmp::xsmpprojectWorkspace();
  // Seed a profile declaration into the workspace index so the `profile`
  // reference has a candidate.
  xsmp::parse(ws, "xsmpproject", "profile 'xsmp-sdk'\n",
              "file:///p/sdk.xsmpprofile");
  (void)cursiv::test::expectCompletion(
      ws.language("xsmpproject"),
      cursiv::test::ExpectedCompletionCallback{
          .text = "project 'P'\nprofile <|>",
          .assertion =
              [](const ::lsp::CompletionList& completion) {
                const ::lsp::CompletionItem* item = nullptr;
                for (const auto& candidate : completion.items) {
                  if (candidate.label == "xsmp-sdk") {
                    item = &candidate;
                  }
                }
                ASSERT_NE(item, nullptr);
                ASSERT_TRUE(item->textEdit.has_value());
                EXPECT_EQ(std::get<::lsp::TextEdit>(*item->textEdit).newText,
                          "'xsmp-sdk'");
                ASSERT_TRUE(item->detail.has_value());
                EXPECT_EQ(*item->detail, "Activated XSMP profile.");
              },
          .parseOptions = xsmp::documentAt("file:///p/xsmp.project"),
          .disposeAfterCheck = true});
}

// Top-level keywords are offered as snippets with placeholders (snippet-only).
TEST(XsmpprojectCompletionProvider, KeywordSnippet) {
  auto ws = xsmp::xsmpprojectWorkspace();
  (void)cursiv::test::expectCompletion(
      ws.language("xsmpproject"),
      cursiv::test::ExpectedCompletionCallback{
          .text = "pro<|>",
          .assertion =
              [](const ::lsp::CompletionList& completion) {
                const ::lsp::CompletionItem* item = nullptr;
                for (const auto& candidate : completion.items) {
                  if (candidate.label == "Project") {
                    item = &candidate;
                  }
                }
                ASSERT_NE(item, nullptr);
                ASSERT_TRUE(item->insertTextFormat.has_value());
                EXPECT_EQ(*item->insertTextFormat,
                          ::lsp::InsertTextFormat::Snippet);
                ASSERT_TRUE(item->textEdit.has_value());
                EXPECT_NE(std::get<::lsp::TextEdit>(*item->textEdit)
                              .newText.find("${1:MissionDemo}"),
                          std::string::npos);
              },
          .parseOptions = xsmp::documentAt("file:///p/xsmp.project"),
          .disposeAfterCheck = true});
}

// After `using`, the supported SMP standard revisions are offered as values.
TEST(XsmpprojectCompletionProvider, StandardValues) {
  auto ws = xsmp::xsmpprojectWorkspace();
  (void)cursiv::test::expectCompletion(
      ws.language("xsmpproject"),
      cursiv::test::ExpectedCompletionCallback{
          .text = "project 'P' using <|>",
          .assertion =
              [](const ::lsp::CompletionList& completion) {
                EXPECT_TRUE(xsmp::hasLabel(completion, "ECSS_SMP_2020"));
                EXPECT_TRUE(xsmp::hasLabel(completion, "ECSS_SMP_2025"));
              },
          .parseOptions = xsmp::documentAt("file:///p/xsmp.project"),
          .disposeAfterCheck = true});
}

TEST(XsmpprojectCompletionProvider,
     StandardsRequireTheProjectHeaderUsingToken) {
  auto world = xsmp::xsmpprojectWorkspace();
  for (const auto text : {
           "project 'P' using 'ECSS_SMP_2020'\n// using <|>",
           "project 'P'\nsource 'using <|>",
           "profile 'sdk' using <|>",
           "project 'P'\nsource 'smdl'\nusing <|>",
           "project 'P' us<|>ing",
       }) {
    SCOPED_TRACE(text);
    (void)cursiv::test::expectCompletion(
        world.language("xsmpproject"),
        cursiv::test::ExpectedCompletionCallback{
            .text = text,
            .assertion =
                [](const auto& list) {
                  EXPECT_FALSE(xsmp::hasLabel(list, "ECSS_SMP_2020"));
                  EXPECT_FALSE(xsmp::hasLabel(list, "ECSS_SMP_2025"));
                },
            .parseOptions = xsmp::documentAt("file:///p/xsmp.project"),
            .disposeAfterCheck = true});
  }
}

TEST(XsmpprojectCompletionProvider,
     StandardsSkipHeaderTriviaAndReplaceIncompleteQuote) {
  auto world = xsmp::xsmpprojectWorkspace();
  for (const auto text : {
           "project 'P' using\n<|>",
           "project 'P' using /* standard */\n<|>",
           "project 'P' using <|>\nsource 'smdl'",
           "project 'P' using 'EC<|>",
           "project 'P' using 'EC<|>'",
       }) {
    SCOPED_TRACE(text);
    ::lsp::CompletionList list;
    std::shared_ptr<cursiv::Document> document;
    (void)cursiv::test::expectCompletion(
        world.language("xsmpproject"),
        cursiv::test::ExpectedCompletionCallback{
            .text = text,
            .assertion =
                [&](const ::lsp::CompletionList& answer) {
                  list = answer;
                  document = world.shared().documents->getDocument(
                      "file:///p/xsmp.project");
                },
            .parseOptions = xsmp::documentAt("file:///p/xsmp.project"),
            .disposeAfterCheck = true});
    ASSERT_TRUE(document);
    EXPECT_TRUE(xsmp::hasLabel(list, "ECSS_SMP_2020"));
    const auto standard = std::ranges::find_if(
        list.items,
        [](const auto& item) { return item.label == "ECSS_SMP_2025"; });
    ASSERT_NE(standard, list.items.end());
    EXPECT_EQ(standard->kind, ::lsp::CompletionItemKind::Value);
    EXPECT_EQ(standard->filterText, "'ECSS_SMP_2025'");
    EXPECT_EQ(standard->sortText, "2000_ECSS_SMP_2025");
    ASSERT_TRUE(standard->textEdit);
    const auto edit = std::get<::lsp::TextEdit>(*standard->textEdit);
    EXPECT_EQ(edit.newText, "'ECSS_SMP_2025'");
    const auto completed = xsmp::applyTextEdits(*document, {edit});
    EXPECT_NE(completed.find("'ECSS_SMP_2025'"), std::string::npos)
        << completed;
    EXPECT_EQ(completed.find("'EC"), completed.rfind("'EC"));
    EXPECT_EQ(completed.find("''ECSS_SMP_2025'"), std::string::npos);
    EXPECT_EQ(completed.find("ECSS_SMP_2025''"), std::string::npos);
  }
}

TEST(XsmpprojectCompletionProvider, ReferenceWritesAQuotedName) {
  auto world = xsmp::xsmpprojectWorkspace();
  (void)xsmp::parse(world, "xsmpproject", "project 'alpha'\n",
                    "file:///alpha/xsmp.project");
  ::lsp::CompletionList list;
  std::shared_ptr<cursiv::Document> document;
  (void)cursiv::test::expectCompletion(
      world.language("xsmpproject"),
      cursiv::test::ExpectedCompletionCallback{
          .text = "project 'P'\ndependency <|>",
          .assertion =
              [&](const ::lsp::CompletionList& answer) {
                list = answer;
                document = world.shared().documents->getDocument(
                    "file:///p/xsmp.project");
              },
          .parseOptions = xsmp::documentAt("file:///p/xsmp.project"),
          .disposeAfterCheck = true});
  ASSERT_TRUE(document);
  const auto item = std::ranges::find_if(
      list.items, [](const auto& it) { return it.label == "alpha"; });
  ASSERT_NE(item, list.items.end());
  EXPECT_EQ(item->filterText, "'alpha'");
  EXPECT_EQ(xsmp::applyTextEdits(*document,
                                 {std::get<::lsp::TextEdit>(*item->textEdit)}),
            "project 'P'\ndependency 'alpha'");
}

TEST(XsmpprojectCompletionProvider, RootAndSourceSnippetPriorities) {
  auto world = xsmp::xsmpprojectWorkspace();
  for (const auto& [text, label] :
       std::array<std::pair<std::string_view, std::string_view>, 2>{
           {{"<|>", "Project"}, {"project 'P'\n<|>", "Source"}}}) {
    (void)cursiv::test::expectCompletion(
        world.language("xsmpproject"),
        cursiv::test::ExpectedCompletionCallback{
            .text = std::string(text),
            .assertion =
                [&](const auto& list) {
                  const auto item = std::ranges::find_if(
                      list.items, [&](const auto& candidate) {
                        return candidate.label == label;
                      });
                  ASSERT_NE(item, list.items.end());
                  EXPECT_EQ(item->filterText, label);
                  EXPECT_EQ(item->sortText, "3000_" + std::string(label));
                },
            .parseOptions = xsmp::documentAt("file:///p/xsmp.project"),
            .disposeAfterCheck = true});
  }
}

TEST(XsmpprojectCompletionProvider,
     StatementsExcludeUsedContributionsAndDependencyCycles) {
  auto world = xsmp::xsmpprojectWorkspace();
  (void)xsmp::parse(world, "xsmpproject", "profile 'sdk'\n",
                    "file:///profiles/sdk.xsmpprofile");
  (void)xsmp::parse(world, "xsmpproject", "tool 'smp'\n",
                    "file:///tools/smp.xsmptool");
  (void)xsmp::parse(world, "xsmpproject", "tool 'python'\n",
                    "file:///tools/python.xsmptool");
  (void)xsmp::parse(world, "xsmpproject", "project 'foundation'\n",
                    "file:///foundation/xsmp.project");
  (void)xsmp::parse(world, "xsmpproject",
                    "project 'cycle'\ndependency 'mission'\n",
                    "file:///cycle/xsmp.project");
  (void)cursiv::test::expectCompletion(
      world.language("xsmpproject"),
      cursiv::test::ExpectedCompletionCallback{
          .text = "project 'mission'\nprofile 'sdk'\ntool 'smp'\n<|>",
          .assertion =
              [](const auto& list) {
                EXPECT_TRUE(xsmp::hasLabel(list, "tool 'python'"));
                EXPECT_TRUE(xsmp::hasLabel(list, "dependency 'foundation'"));
                EXPECT_FALSE(xsmp::hasLabel(list, "profile 'sdk'"));
                EXPECT_FALSE(xsmp::hasLabel(list, "tool 'smp'"));
                EXPECT_FALSE(xsmp::hasLabel(list, "dependency 'cycle'"));
                EXPECT_FALSE(xsmp::hasLabel(list, "dependency 'mission'"));
              },
          .parseOptions = xsmp::documentAt("file:///mission/xsmp.project"),
          .disposeAfterCheck = true});
}

TEST(XsmpprojectCompletionProvider,
     RegistryDefaultsCanonicalUsageAndStatementDocumentation) {
  auto world = xsmp::xsmpprojectWorkspace();
  auto registry = std::make_shared<xsmp::Registry>();
  xsmp::Contribution first;
  first.id = "alpha";
  first.kind = "tool";
  first.description = "Alpha support.";
  registry->add(std::move(first));
  xsmp::Contribution preferred;
  preferred.id = "preferred";
  preferred.kind = "tool";
  preferred.defaultSelected = true;
  preferred.aliases = {"legacy"};
  registry->add(std::move(preferred));
  world.shared().contributionRegistry = registry;
  (void)xsmp::parse(world, "xsmpproject", "tool 'ghost'",
                    "file:///ghost.xsmptool");
  (void)cursiv::test::expectCompletion(
      world.language("xsmpproject"),
      cursiv::test::ExpectedCompletionCallback{
          .text = "<|>",
          .assertion =
              [](const auto& list) {
                const auto item =
                    std::ranges::find_if(list.items, [](const auto& candidate) {
                      return candidate.label == "Tool";
                    });
                ASSERT_NE(item, list.items.end());
                EXPECT_EQ(item->insertText, "tool '${1:preferred}'");
                EXPECT_FALSE(xsmp::hasLabel(list, "tool"));
              },
          .parseOptions = xsmp::documentAt("file:///root.xsmptool"),
          .disposeAfterCheck = true});
  (void)cursiv::test::expectCompletion(
      world.language("xsmpproject"),
      cursiv::test::ExpectedCompletionCallback{
          .text = "project 'Demo'\ntool 'legacy'\n<|>",
          .assertion =
              [](const auto& list) {
                EXPECT_TRUE(xsmp::hasLabel(list, "Source"));
                EXPECT_FALSE(xsmp::hasLabel(list, "tool 'preferred'"));
                EXPECT_FALSE(xsmp::hasLabel(list, "tool 'legacy'"));
                EXPECT_FALSE(xsmp::hasLabel(list, "tool 'ghost'"));
                const auto item =
                    std::ranges::find_if(list.items, [](const auto& candidate) {
                      return candidate.label == "tool 'alpha'";
                    });
                ASSERT_NE(item, list.items.end());
                ASSERT_TRUE(item->documentation);
                EXPECT_EQ(
                    std::get<::lsp::MarkupContent>(*item->documentation).value,
                    "Enable the installed XSMP tool `alpha`.\n\nAlpha "
                    "support.");
              },
          .parseOptions = xsmp::documentAt("file:///demo/xsmp.project"),
          .disposeAfterCheck = true});
}

TEST(XsmpprojectCompletionProvider, ContractCases) {
  using namespace xsmp;
  const auto fixture =
      ::lsp::json::parse(readText(std::filesystem::path(XSMP_FIXTURES_DIR)
                                  / "lsp/project-completion-contracts.json"));
  for (const auto& entry : fixture.object().get("cases").array()) {
    const auto& record = entry.object();
    SCOPED_TRACE(stringAt(record, "name"));
    auto world = xsmp::xsmpprojectWorkspace();
    auto registry = std::make_shared<Registry>();
    for (const auto id :
         {"smp", "adoc", "python", "xsmp-sdk", "esa-cdk", "tas-mdk"}) {
      Contribution contribution;
      contribution.id = id;
      contribution.kind = std::string_view(id).ends_with("-sdk")
                                  || std::string_view(id).ends_with("-cdk")
                                  || std::string_view(id).ends_with("-mdk")
                              ? "profile"
                              : "tool";
      contribution.defaultSelected = std::string_view(id) == "xsmp-sdk";
      registry->add(std::move(contribution));
    }
    world.shared().contributionRegistry = registry;
    for (const auto& background : record.get("documents").array())
      (void)xsmp::parse(world, "xsmpproject",
                        stringAt(background.object(), "text"),
                        stringAt(background.object(), "uri"));
    const auto uri = stringAt(record, "uri");
    ::lsp::CompletionList list;
    std::shared_ptr<cursiv::Document> document;
    (void)cursiv::test::expectCompletion(
        world.language("xsmpproject"),
        cursiv::test::ExpectedCompletionCallback{
            .text = stringAt(record, "text"),
            .assertion =
                [&](const ::lsp::CompletionList& answer) {
                  list = answer;
                  document = world.shared().documents->getDocument(uri);
                },
            .parseOptions = xsmp::documentAt(uri),
            .disposeAfterCheck = true});
    ASSERT_TRUE(document);
    for (const auto& item : record.get("assertions").array()) {
      const auto& assertion = item.object();
      const auto label = stringAt(assertion, "label"),
                 kind = stringAt(assertion, "kind");
      SCOPED_TRACE(kind + ": " + label);
      if (kind == "present")
        EXPECT_TRUE(xsmp::hasLabel(list, label));
      else if (kind == "absent")
        EXPECT_FALSE(xsmp::hasLabel(list, label));
      else if (kind == "insertContains") {
        const auto completion = std::ranges::find_if(
            list.items,
            [&](const auto& value) { return value.label == label; });
        ASSERT_NE(completion, list.items.end());
        const auto text = xsmp::completionText(*completion);
        EXPECT_NE(text.find(stringAt(assertion, "value")), text.npos);
      }
    }
  }
}
