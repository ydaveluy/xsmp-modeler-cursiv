#include <gtest/gtest.h>

#include <filesystem>
#include <string>
#include <xsmp/Json.hpp>
#include <xsmp/XsmpprojectLspModule.hpp>

#include "support/Files.hpp"
#include "support/LspTestSupport.hpp"

// The cases of `formatting-options-contracts.json` give what xsmp-modeler
// gives, but for one deliberate difference: a selection formats every line it
// holds, also in a construction that starts before it. xsmp-modeler leaves such
// a construction alone, so the inside of a `configure` or `task` block that
// holds only the start of the selection stays as it was there (the xsmpasb and
// xsmpsed cases with a range).
TEST(XsmpcatFormatter, OptionsAndSelectionContracts) {
  using namespace xsmp;
  const auto fixture = ::lsp::json::parse(
      xsmp::readFile(std::filesystem::path(XSMP_FIXTURES_DIR)
                     / "lsp/formatting-options-contracts.json"));
  ASSERT_EQ(fixture.object().get("cases").array().size(), 36U);
  for (const auto& value : fixture.object().get("cases").array()) {
    const auto& row = value.object();
    SCOPED_TRACE(stringAt(row, "name"));
    auto ws = xsmp::xsmpcatWorkspace();
    auto& shared = ws.shared();
    ws.registerLanguage(xsmp::createXsmpprojectServices(shared));
    ASSERT_TRUE(xsmp::registerXsmpasbServices(shared));
    ASSERT_TRUE(xsmp::registerXsmpcfgServices(shared));
    ASSERT_TRUE(xsmp::registerXsmplnkServices(shared));
    ASSERT_TRUE(xsmp::registerXsmpsedServices(shared));
    const auto language = stringAt(row, "language");
    ::lsp::FormattingOptions options;
    options.tabSize = row.get("options").object().get("tabSize").integer();
    options.insertSpaces =
        row.get("options").object().get("insertSpaces").boolean();
    std::optional<::lsp::Range> range;
    if (const auto* selection = row.find("range")) {
      const auto position = [](const Json& value) {
        const auto& p = value.object();
        return ::lsp::Position{
            .line = static_cast<std::uint32_t>(p.get("line").integer()),
            .character =
                static_cast<std::uint32_t>(p.get("character").integer())};
      };
      range = ::lsp::Range{.start = position(selection->object().get("start")),
                           .end = position(selection->object().get("end"))};
    }
    // A project document is known by its file name.
    (void)cursiv::test::expectFormatting(
        ws.language(language),
        {.before = stringAt(row, "before"),
         .after = stringAt(row, "after"),
         .parseOptions = xsmp::documentAt(
             language == "xsmpproject" ? "file:///format/xsmp.project"
                                       : "file:///format/case." + language),
         .range = range,
         .options = options});
  }
}

// Formatting `not-formatted.xsmpcat` must reproduce `formatted.xsmpcat` byte
// for byte.
TEST(XsmpcatFormatter, FormatsReferenceFileExactly) {
  auto ws = xsmp::xsmpcatWorkspace();
  const std::filesystem::path fixtures = XSMP_FIXTURES_DIR;
  (void)cursiv::test::expectFormatting(
      ws.language("xsmpcat"),
      {.before = xsmp::readFile(fixtures / "lsp/not-formatted.xsmpcat"),
       .after = xsmp::readFile(fixtures / "lsp/formatted.xsmpcat")});
}

// Shorthand multiplicities collapse: `Sensor   *   sensors` becomes
// `Sensor* sensors` and `ILogger   +   loggers` becomes `ILogger+ loggers` (no
// space before the multiplicity marker, one space after).
TEST(XsmpcatFormatter, ShorthandMultiplicityNoExtraSpaces) {
  auto ws = xsmp::xsmpcatWorkspace();
  (void)cursiv::test::expectFormatting(
      ws.language("xsmpcat"),
      {.before = "catalogue test\n"
                 "namespace demo\n"
                 "{\n"
                 "    model Platform\n"
                 "    {\n"
                 "        container    Sensor   *   sensors\n"
                 "        reference    ILogger   +   loggers\n"
                 "    }\n"
                 "}\n",
       .after = "catalogue test\n"
                "namespace demo\n"
                "{\n"
                "    model Platform\n"
                "    {\n"
                "        container Sensor* sensors\n"
                "        reference ILogger+ loggers\n"
                "    }\n"
                "}\n"});
}

TEST(XsmpcatFormatter, EntryPointBodyAndItsSections) {
  auto ws = xsmp::xsmpcatWorkspace();
  (void)cursiv::test::expectFormatting(
      ws.language("xsmpcat"),
      {.before = "catalogue c\nnamespace n\n{\n    model M\n    {\n        "
                 "entrypoint go  {  in  a out   b }\n    }\n}\n",
       .after = "catalogue c\nnamespace n\n{\n    model M\n    {\n        "
                "entrypoint go\n        {\n            in a\n            out "
                "b\n        }\n    }\n}\n"});
  // What is already laid out stays so, with the comment on its section.
  const std::string laidOut =
      "catalogue c\nnamespace n\n{\n    model M\n    {\n        entrypoint "
      "go\n        {\n            /** The input. */\n            in a\n"
      "            out b\n        }\n    }\n}\n";
  (void)cursiv::test::expectFormatting(ws.language("xsmpcat"),
                                       {.before = laidOut, .after = laidOut});
}

TEST(XsmpcatFormatter, AttributeStaysOnTheLineItWasWrittenOn) {
  auto ws = xsmp::xsmpcatWorkspace();
  const std::string head = "catalogue c\nnamespace n\n{\n    ";
  (void)cursiv::test::expectFormatting(
      ws.language("xsmpcat"),
      {.before = head + "@Attr(1)   private primitive P\n}\n",
       .after = head + "@Attr(1) private primitive P\n}\n"});
  (void)cursiv::test::expectFormatting(
      ws.language("xsmpcat"),
      {.before = head + "@Attr(1)\n  private primitive P\n}\n",
       .after = head + "@Attr(1)\n    private primitive P\n}\n"});
}

TEST(XsmpcatFormatter, BuiltInNamesFollowTheirDollar) {
  auto ws = xsmp::xsmpcatWorkspace();
  const std::string head =
      "catalogue c\nnamespace n\n{\n    struct S\n    {\n        constant "
      "Smp.Float64 ";
  (void)cursiv::test::expectFormatting(ws.language("xsmpcat"),
                                       {.before = head + "a = $ PI\n    }\n}\n",
                                        .after = head + "a = $PI\n    }\n}\n"});
  (void)cursiv::test::expectFormatting(
      ws.language("xsmpcat"), {.before = head + "a = $ sin ( 1 )\n    }\n}\n",
                               .after = head + "a = $sin(1)\n    }\n}\n"});
}
