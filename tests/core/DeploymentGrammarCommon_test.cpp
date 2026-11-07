#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <tuple>

#include "support/DeploymentWorld.hpp"

TEST(DeploymentGrammarCommon, CarriageReturnLineFeedIsABlank) {
  xsmp::DeploymentWorld world;
  xsmp::expectClean(
      world.parse("xsmpasb", "assembly A\r\nRoot: \"External\" {}\r\n"));
}

TEST(DeploymentGrammarCommon, ExampleCorpusParsesAllFourExtensions) {
  xsmp::DeploymentWorld world;
  const auto path =
      std::filesystem::path(XSMP_FIXTURES_DIR) / "workspace" / "examples";
  ASSERT_TRUE(std::filesystem::exists(path)) << path;
  std::size_t count = 0;
  for (const auto& file : std::filesystem::recursive_directory_iterator(path)) {
    const auto extension = file.path().extension().string();
    if (extension != ".xsmpasb" && extension != ".xsmpcfg"
        && extension != ".xsmplnk" && extension != ".xsmpsed")
      continue;
    std::ifstream stream(file.path());
    const std::string source{std::istreambuf_iterator<char>(stream), {}};
    auto result = world.parse(extension.substr(1), source);
    SCOPED_TRACE(file.path().string());
    xsmp::expectClean(result);
    ++count;
  }
  EXPECT_GE(count, 12u);
}

// xsd:float and xsd:double take a sign, in every language that has values.
TEST(DeploymentGrammarCommon, FloatsTakeASign) {
  xsmp::DeploymentWorld world;
  const auto firstFloat = [](const cursiv::ParseResult& result) {
    double value = 0;
    for (const auto* node : result.nodes())
      if (const auto* number = cursiv::ast_cast<xsmp::FloatValue>(node))
        value = number->value;
      else if (const auto* typed = cursiv::ast_cast<xsmp::Float64Value>(node))
        value = typed->value;
    return value;
  };
  for (const auto& [text, expected] :
       std::initializer_list<std::pair<std::string_view, double>>{
           {"-1.5", -1.5},
           {"+1.5", 1.5},
           {"-.5", -0.5},
           {"-1e2", -100.0},
           {"-1'000.5", -1000.5},
           {"-1.5f64", -1.5}}) {
    SCOPED_TRACE(text);
    const auto value = std::string(text);
    auto configuration =
        world.parse("xsmpcfg", "configuration C /m: n.M { x = " + value + " }");
    xsmp::expectClean(configuration);
    EXPECT_EQ(firstFloat(configuration), expected);
    auto assembly =
        world.parse("xsmpasb", "assembly A Root: n.M { x = " + value + " }");
    xsmp::expectClean(assembly);
    EXPECT_EQ(firstFloat(assembly), expected);
  }
  xsmp::expectClean(world.parse("xsmpasb", "assembly A Root: n.M { x = +1 }"));
}

// `INF` and `NaN` are not numbers here: they read as names, which no literal
// answers, so a value is never silently lost.
TEST(DeploymentGrammarCommon, InfinityAndNanAreNotNumbers) {
  xsmp::DeploymentWorld world;
  for (const auto* word : {"INF", "NaN"}) {
    const auto result = world.parse(
        "xsmpcfg", "configuration C /m: n.M { x = " + std::string(word) + " }");
    xsmp::expectClean(result);
    auto found = false;
    for (const auto* node : result.nodes())
      if (const auto* name = cursiv::ast_cast<xsmp::EnumerationValue>(node))
        found = name->reference.refText == word;
    EXPECT_TRUE(found) << word;
  }
  EXPECT_FALSE(world.parse("xsmpcfg", "configuration C /m: n.M { x = -INF }")
                   .diagnostics.empty());
}

TEST(DeploymentGrammarCommon, StringsAndCharactersReadTheEscapesTheyDecode) {
  xsmp::DeploymentWorld world;
  const auto value = [](const cursiv::ParseResult& result) {
    std::string text;
    for (const auto* node : result.nodes())
      if (const auto* string = cursiv::ast_cast<xsmp::String8Value>(node))
        text = string->value;
      else if (const auto* character = cursiv::ast_cast<xsmp::Char8Value>(node))
        text = character->value;
    return text;
  };
  for (const auto& [literal, expected] :
       std::initializer_list<std::pair<std::string_view, std::string_view>>{
           {R"("a\"b")", "a\"b"},
           {R"("C:\\")", "C:\\"},
           {R"("\x41\n")", "A\n"},
           {R"('\'')", "'"},
           {R"('\\')", "\\"},
           {R"("say 'hi'")", "say 'hi'"}}) {
    SCOPED_TRACE(literal);
    const auto result = world.parse(
        "xsmpcfg",
        "configuration C /m: n.M { x = " + std::string(literal) + " }");
    xsmp::expectClean(result);
    EXPECT_EQ(value(result), expected);
  }
  // A real line break never stands in a literal: `\n` writes one.
  for (const auto& [language, before, after] : std::initializer_list<
           std::tuple<std::string_view, std::string_view, std::string_view>>{
           {"xsmpcfg", "configuration C /m: n.M { x = \"", "\" }"},
           {"xsmpcfg", "configuration C /m: n.M { x = '", "' }"},
           {"xsmpcfg", "configuration C /m: n.M { x = \"", "\"d }"},
           {"xsmpcfg", "configuration C /m: n.M { x = \"", "\"dt }"},
           {"xsmpasb", "assembly <R = \"", "\"> A Root: \"n::M\""},
           {"xsmpsed", "schedule <R = \"", "\"> S"},
       }) {
    const auto parsed = [&, language = language, before = before,
                         after = after](std::string_view text) {
      return world.parse(language, std::string(before) + std::string(text)
                                       + std::string(after));
    };
    SCOPED_TRACE(before);
    xsmp::expectClean(parsed(R"(a\nb)"));
    for (const auto* text : {"a\nb", "a\rb", "a\r\nb", "a\\\nb", "a\\\rb"})
      EXPECT_FALSE(parsed(text).diagnostics.empty()) << text;
  }
  // A link base has no string at all.
  for (const auto* text : {"\"ab\"", "\"a\nb\""})
    EXPECT_FALSE(
        world.parse("xsmplnk", "link L / { " + std::string(text) + " }")
            .diagnostics.empty());

  // The backslash escapes the quote, as in C, so this string never closes.
  EXPECT_FALSE(
      world.parse("xsmpcfg", R"(configuration C /m: n.M { x = "C:\" })")
          .diagnostics.empty());
  xsmp::expectClean(
      world.parse("xsmpcfg", R"(configuration C /m: n.M { a = "PT1S"d b = "x\"y"
    c = "2000-01-01T00:00:00Z"dt })"));
}
