#include <gtest/gtest.h>

#include <algorithm>
#include <cursiv/workspace/Document.hpp>
#include <cursiv/workspace/DocumentBuilder.hpp>
#include <cursiv/workspace/Documents.hpp>
#include <numeric>
#include <xsmp/CliWorkspace.hpp>
#include <xsmp/Json.hpp>
#include <xsmp/utils/XsmpUtils.hpp>

#include "core/validation/DeploymentCases.hpp"
#include "core/validation/DeploymentSemanticsFixture.hpp"

namespace {

using namespace xsmp;

const Json& contracts() {
  static const auto value = ::lsp::json::parse(
      readText(std::filesystem::path(XSMP_FIXTURES_DIR)
               / "core/deployment-diagnostic-contracts.json"));
  return value;
}

Json range(const cursiv::Document& document, cursiv::TextRange offsets) {
  const auto begin = document.textDocument().positionAt(offsets.begin);
  const auto end = document.textDocument().positionAt(offsets.end);
  return JsonArray{JsonArray{begin.line, begin.character},
                   JsonArray{end.line, end.character}};
}

Json diagnostic(const cursiv::Diagnostic& issue,
                const cursiv::Document& document,
                const cursiv::SharedCoreServices& shared) {
  JsonObject value{{"severity", static_cast<int>(issue.severity)},
                   {"message", issue.message},
                   {"range", range(document, issue.range)}};
  if (issue.data)
    value.insert("data", ::lsp::json::parse(issue.data->toJsonString()));
  if (issue.code
      && !(issue.code->isString() && issue.data && issue.data->isObject()
           && issue.data->object().contains("code")
           && issue.data->object().at("code").isString()
           && issue.data->object().at("code").string() == issue.code->string()))
    std::visit([&](const auto& code) { value.insert("code", code); },
               issue.code->value);
  if (issue.codeDescription)
    value.insert("codeDescription", *issue.codeDescription);
  if (!issue.tags.empty()) {
    JsonArray tags;
    for (const auto tag : issue.tags) tags.emplace_back(static_cast<int>(tag));
    value.insert("tags", std::move(tags));
  }
  if (!issue.relatedInformation.empty()) {
    JsonArray related;
    for (const auto& item : issue.relatedInformation) {
      const auto other = shared.documents->getDocument(item.uri);
      related.emplace_back(
          JsonObject{{"uri", item.uri},
                     {"message", item.message},
                     {"range", range(other ? *other : document, item.range)}});
    }
    value.insert("relatedInformation", std::move(related));
  }
  return value;
}

std::vector<std::size_t> diagnosticIndices() {
  std::vector<std::size_t> result(xsmp::deploymentCases().size());
  std::iota(result.begin(), result.end(), 0);
  return result;
}

class DeploymentValidatorDiagnostics
    : public ::testing::TestWithParam<std::size_t> {};

}  // namespace

TEST_P(DeploymentValidatorDiagnostics, MatchesTheFullDiagnosticContract) {
  const auto& scenario = xsmp::deploymentCases()[GetParam()];
  SCOPED_TRACE(scenario.lang + ": " + scenario.name);
  const auto& golden =
      contracts().object().get("cases").array()[GetParam()].object();
  ASSERT_EQ(golden.get("index").integer(), GetParam());
  ASSERT_EQ(golden.get("name").string(), scenario.name);
  auto shared = xsmp::createXsmpSharedServices();
  xsmp::registerLanguages(*shared);
  std::vector<cursiv::Document*> documents;
  for (const auto& input : scenario.docs) {
    auto document = shared->documents->createDocument(
        "file:///cases/" + input.first, input.second);
    documents.push_back(document.get());
  }
  const auto target = shared->documents->createDocument(
      "file:///cases/src/target." + scenario.lang, scenario.text);
  documents.push_back(target.get());
  shared->documentBuilder->build(documents);
  ASSERT_TRUE(target->parseSucceeded());
  auto expected = golden.get("diagnostics").array();
  JsonArray actual;
  for (const auto& issue : target->diagnostics)
    actual.push_back(diagnostic(issue, *target, *shared));
  // Compare a multiset of full JSON objects, retaining duplicate diagnostics.
  ASSERT_EQ(actual.size(), expected.size());
  for (const auto& item : actual) {
    const auto found = std::ranges::find(expected, item);
    ASSERT_NE(found, expected.end())
        << "Unexpected diagnostic: " << ::lsp::json::stringify(item);
    expected.erase(found);
  }
  EXPECT_TRUE(expected.empty());
}

INSTANTIATE_TEST_SUITE_P(Cases, DeploymentValidatorDiagnostics,
                         ::testing::ValuesIn(diagnosticIndices()),
                         [](const ::testing::TestParamInfo<std::size_t>& info) {
                           return xsmp::deploymentCases()[info.param].lang + "_"
                                  + std::to_string(info.param);
                         });

namespace {

// The messages, severities and counts expected for each deployment case. The
// order of the diagnostics is not significant: exact expectations compare
// multisets, retaining duplicate counts and every expected and extra
// diagnostic.
using xsmp::deploymentCases;

std::vector<std::size_t> caseIndices() {
  std::vector<std::size_t> result(deploymentCases().size());
  std::iota(result.begin(), result.end(), 0);
  return result;
}

bool matchesFilter(std::string_view message, const xsmp::Assertion& assertion) {
  if (assertion.filterValues.empty()) return true;
  if (assertion.filterContains)
    return std::ranges::all_of(assertion.filterValues, [&](const auto& text) {
      return message.find(text) != std::string_view::npos;
    });
  return message == assertion.filterValues[0];
}

class DeploymentValidatorCases : public ::testing::TestWithParam<std::size_t> {
};

}  // namespace

TEST_P(DeploymentValidatorCases, MeetsItsExpectations) {
  const auto& scenario = deploymentCases()[GetParam()];
  const auto language = scenario.lang;
  SCOPED_TRACE(language + ": " + scenario.name);
  auto shared = xsmp::createXsmpSharedServices();
  xsmp::registerLanguages(*shared);
  std::vector<std::shared_ptr<cursiv::Document>> documents;
  for (const auto& input : scenario.docs)
    documents.push_back(shared->documents->createDocument(
        "file:///cases/" + input.first, input.second));
  auto target = shared->documents->createDocument(
      "file:///cases/src/target." + language, scenario.text);
  documents.push_back(target);
  std::vector<cursiv::Document*> raw;
  for (const auto& document : documents) {
    ASSERT_TRUE(document->parseSucceeded()) << document->uri;
    raw.push_back(document.get());
  }
  shared->documentBuilder->build(raw);
  std::vector<std::string> messages;
  for (const auto& diagnostic : target->diagnostics)
    messages.push_back(diagnostic.message);
  for (const auto& assertion : scenario.assertions) {
    const auto mode = assertion.mode;
    std::vector<std::string> actual;
    for (const auto& diagnostic : target->diagnostics) {
      if (!matchesFilter(diagnostic.message, assertion)) continue;
      if (assertion.diagnostics)
        actual.push_back(std::to_string(static_cast<int>(diagnostic.severity))
                         + ":" + diagnostic.message);
      else
        actual.push_back(diagnostic.message);
    }
    if (mode == "count") {
      EXPECT_EQ(actual.size(), assertion.count);
      continue;
    }
    if (mode == "none" || mode == "some") {
      EXPECT_EQ(!actual.empty(), mode == "some");
      continue;
    }
    auto expected = assertion.values;
    if (mode == "exact") {
      std::ranges::sort(actual);
      std::ranges::sort(expected);
      EXPECT_EQ(actual, expected);
    } else {
      for (const auto& message : expected)
        EXPECT_EQ(std::ranges::find(actual, message) != actual.end(),
                  mode == "contains")
            << message;
    }
  }
  if (scenario.name ==
      "navigates to templated child instances from a concrete configuration "
      "path") {
    const auto* assembly =
        cursiv::ast_cast<xsmp::Assembly>(target->parseResult.value);
    ASSERT_NE(assembly, nullptr);
    ASSERT_FALSE(assembly->configurations.empty());
    const auto* head = cursiv::ast_cast<xsmp::ConcretePathNamedSegment>(
        assembly->configurations[0]->name->head);
    ASSERT_NE(head, nullptr);
    ASSERT_NE(head->reference.get(), nullptr);
    EXPECT_EQ(head->reference.get()->name, "Unit{Lane}");
  }
}

INSTANTIATE_TEST_SUITE_P(Cases, DeploymentValidatorCases,
                         ::testing::ValuesIn(caseIndices()),
                         [](const ::testing::TestParamInfo<std::size_t>& info) {
                           return deploymentCases()[info.param].lang + "_"
                                  + std::to_string(info.param);
                         });

using DeploymentValidatorTest = DeploymentSemantics;

namespace {

constexpr std::string_view notAName = "A template value shall hold only";

}  // namespace

// `Assembly:Name` of the SMP schema.
TEST_F(DeploymentValidatorTest, TemplateValuesAreNamesOfTheSchema) {
  for (const auto* value :
       {"Root", "", "a_1", "Sys{Lane}", "{Lane}_{Other}", "{}"})
    EXPECT_FALSE(
        has(*build("xsmpasb", "assembly <Lane = \"" + std::string(value)
                                  + "\"> Good\nRoot: demo.Root\n"),
            notAName))
        << value;
  for (const auto* value : {"/System", "a b", "a-b", "a.b", "{", "}", "a{b",
                            "{a b}", "{{a}}", "\\x01", "é"}) {
    EXPECT_TRUE(has(*build("xsmpasb", "assembly <Lane = \"" + std::string(value)
                                          + "\"> Bad\nRoot: demo.Root\n"),
                    notAName))
        << value;
    EXPECT_TRUE(has(*build("xsmpsed", "schedule <Root = \"" + std::string(value)
                                          + "\"> Plan\n"),
                    notAName))
        << value;
  }

  // An argument of an assembly instance, and of a task that another executes.
  EXPECT_TRUE(has(*build("xsmpasb", R"(assembly Parent
Root: demo.Root { child += Sub: DemoAsm<Lane = "a/b"> }
)"),
                  notAName));
  EXPECT_FALSE(has(*build("xsmpasb", R"(assembly GoodParent
Root: demo.Root { child += Sub: DemoAsm<Lane = "ab"> }
)"),
                   notAName));
  EXPECT_TRUE(has(*build("xsmpsed", R"(schedule <Root = "root"> Plan
task Main { execute Other<Root = "a/b"> }
task Other { emit "A" }
)"),
                  notAName));
}

// Where SMP XML has a start index: a simple array of a Configuration of ECSS
// SMP 2025.
TEST_F(DeploymentValidatorTest, AStartIndexStandsOnlyWhereTheStandardHasOne) {
  add("src/simple.xsmpcat", R"(catalogue Simple namespace simple {
 public attribute Smp.Bool Marker
 @Attributes.SimpleArray public array Quad = Smp.Int32[4]
 public array Plain = Smp.Int32[4]
 public model M { field Quad quad field Plain plain }
})");
  clean(*build("xsmpcfg", R"(configuration Good
/M: simple.M { quad = [1: 2i32, 3i32] }
)"));
  EXPECT_TRUE(has(*build("xsmpcfg", R"(configuration NotSimple
/M: simple.M { plain = [1: 2i32, 3i32] }
)"),
                  "only allowed for SimpleArray"));

  // Nothing gives these a type, so their items do.
  const auto unsafeValue = [&](std::string_view array) {
    return build("xsmpcfg", "configuration U" + std::to_string(documents.size())
                                + "\n/M { unsafe a = " + std::string(array)
                                + " }\n");
  };
  clean(*unsafeValue("[2: 3i32, 4i32]"));
  EXPECT_TRUE(
      has(*unsafeValue("[2: 3i32, 4.5f64]"), "only allowed for SimpleArray"));
  EXPECT_TRUE(has(*unsafeValue("[2: 3, 4]"), "only allowed for SimpleArray"));
  EXPECT_TRUE(has(*unsafeValue("[2: ]"), "only allowed for SimpleArray"));

  // An assembly has none, whatever the array.
  EXPECT_TRUE(has(*build("xsmpasb", R"(assembly HasNone
M: simple.M { quad = [1: 2i32, 3i32] }
)"),
                  "only available in a Configuration"));
}

TEST(DeploymentValidator, AStartIndexNeedsTheStandardOf2025) {
  auto shared = xsmp::createXsmpSharedServices();
  xsmp::registerLanguages(*shared);
  const auto project = shared->documents->createDocument(
      "file:///old/xsmp.project",
      "project 'Old' using 'ECSS_SMP_2020'\nsource 'src'\n");
  const auto document = shared->documents->createDocument(
      "file:///old/src/c.xsmpcfg",
      "configuration C\n/M { unsafe a = [2: 3i32] }\n");
  std::vector<cursiv::Document*> raw{project.get(), document.get()};
  shared->documentBuilder->build(raw);
  EXPECT_TRUE(DeploymentSemantics::has(
      *document, "StartIndex is only available in ECSS_SMP_2025."));
}

// Any character is a valid value, whatever XML can hold.
TEST_F(DeploymentValidatorTest, ValuesHoldAnyCharacter) {
  const auto valueIs = [&](std::string_view value) {
    return build("xsmpcfg", "configuration V" + std::to_string(documents.size())
                                + "\n/M { unsafe a = " + std::string(value)
                                + " }\n");
  };
  clean(*valueIs(R"("\x01")"));
  clean(*valueIs(R"("a\0b")"));
  clean(*valueIs(R"('\0')"));
  clean(*valueIs(R"('\x1F')"));
  clean(*valueIs(R"("\uFFFE")"));

  EXPECT_FALSE(has(*build("xsmpasb", R"(assembly Sends
configure Child { subscribe step -> "\x01" }
Root: demo.Root
)"),
                   "character"));
  EXPECT_FALSE(has(*build("xsmpasb", R"(assembly Named
Root: "demo.\x01"
)"),
                   "character"));
  EXPECT_FALSE(has(*build("xsmpsed", R"(schedule <Root = "root"> Sets
task Main { property unsafe a = "\x01" }
)"),
                   "character"));
}
