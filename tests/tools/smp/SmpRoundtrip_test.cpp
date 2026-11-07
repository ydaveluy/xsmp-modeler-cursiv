#include <gtest/gtest.h>

#include <algorithm>
#include <cursiv/services/SharedCoreServices.hpp>
#include <cursiv/workspace/DocumentBuilder.hpp>
#include <cursiv/workspace/DocumentFactory.hpp>
#include <cursiv/workspace/Documents.hpp>
#include <cursiv/workspace/IndexManager.hpp>
#include <filesystem>
#include <pugixml.hpp>
#include <string>
#include <xsmp/CliWorkspace.hpp>
#include <xsmp/SharedCoreServices.hpp>
#include <xsmp/Traversal.hpp>
#include <xsmp/smp/SmpService.hpp>
#include <xsmp/smp/SmpXmlText.hpp>
#include <xsmp/utils/Utf8.hpp>

#include "support/EscapeCases.hpp"
#include "support/Files.hpp"
#include "tools/smp/SmpWorld.hpp"

// Whether XML 1.0 can hold `text` as it is.
[[nodiscard]] static bool xmlCanHold(std::string_view text) {
  for (std::size_t i = 0; i < text.size();) {
    const auto size = xsmp::utf8CharacterSize(text.substr(i));
    const auto byte = static_cast<unsigned char>(text[i]);
    if (size == 0
        || (byte < 0x20 && byte != '\t' && byte != '\n' && byte != '\r'))
      return false;
    i += size;
  }
  return text.find("\xEF\xBF\xBE") == text.npos
         && text.find("\xEF\xBF\xBF") == text.npos;
}

// Whatever the text, what is decoded is what was encoded.
TEST(SmpRoundtrip, EveryTextSurvivesTheEscape) {
  const auto check = [](std::string_view text) {
    const auto written = xsmp::encodeXmlText(text);
    EXPECT_EQ(xsmp::decodeXmlText(written), text) << written;
    EXPECT_TRUE(xmlCanHold(written)) << written;
  };
  // Every short mix of what an escape is made of.
  constexpr std::string_view pieces[] = {
      "\\", "x",    "0",    "1",    "5",    "C",  "E",
      "\0", "\x01", "\xFF", "\xC3", "\xA9", "\n", "\xEF\xBF\xBE"};
  std::vector<std::string> texts{""};
  for (auto length = 0; length < 4; ++length) {
    const auto count = texts.size();
    for (std::size_t i = 0; i < count; ++i)
      for (const auto piece : pieces)
        texts.push_back(texts[i] + std::string(piece));
  }
  for (const auto& text : texts) check(text);
  for (const auto* text :
       {R"(\x00)", R"(\x5C)", R"(\x5c)", R"(\\x00)", R"(\xEF\xBF\xBE)",
        R"(\xef\xbf\xbf)", R"(\xEF\xBF\xBD)", R"(\xEF\xBF)", R"(\xEF)",
        R"(\xFF)", R"(\xff)", R"(\xC3\xA9)"})
    check(text);
  for (auto byte = 0; byte < 256; ++byte) {
    const auto text = std::string(1, static_cast<char>(byte));
    check(text);
    check("\\" + text);
    check(R"(\x)" + text);
    check(R"(\x0)" + text);
  }
}

TEST(SmpRoundtrip, TextsStayAsTheyAreWhenXmlCanHoldThem) {
  for (const auto* text : {"", "plain", R"(C:\dir)", R"(a\nb)", R"(\x41)",
                           R"(\xZZ)", R"(\x0)", R"(\)", "tab\t line\n return\r",
                           "caf\xC3\xA9", "\x7F", "\xEF\xBF\xBD"}) {
    EXPECT_EQ(xsmp::encodeXmlText(text), text);
    EXPECT_EQ(xsmp::decodeXmlText(text), text);
  }
}

// The bytes of a text are not all UTF-8, and each comes back.
TEST(SmpRoundtrip, BytesThatAreNotUtf8AreWrittenWithAnEscape) {
  EXPECT_EQ(xsmp::encodeXmlText("a\xFF"
                                "b"),
            R"(a\xFFb)");
  EXPECT_EQ(xsmp::encodeXmlText("\xC3"), R"(\xC3)");
  EXPECT_EQ(xsmp::encodeXmlText("\xC3\xA9"), "\xC3\xA9");
  EXPECT_EQ(xsmp::encodeXmlText("\xE2\x82"), R"(\xE2\x82)");
  EXPECT_EQ(xsmp::encodeXmlText("\xED\xA0\x80"), R"(\xED\xA0\x80)");
  // Text that reads as such an escape keeps its backslash.
  EXPECT_EQ(xsmp::encodeXmlText(R"(\xFF)"), R"(\x5CxFF)");
  EXPECT_EQ(xsmp::decodeXmlText(R"(\xC3\xA9)"), "\xC3\xA9");
  EXPECT_EQ(xsmp::decodeXmlText(R"(\xe9)"), "\xE9");
}

// Every literal of the table in a configuration: what XML holds is its value,
// and the import gives back the same value in the same spelling.
TEST(SmpRoundtrip, EveryEscapeOfAConfigurationComesBack) {
  std::string source = "configuration C /Root {\n";
  std::vector<const xsmp::escapes::Case*> cases;
  for (const auto& c : xsmp::escapes::cases()) {
    if (c.isConcatenated()) continue;
    source += "k" + std::to_string(cases.size()) + " = " + std::string(c.source)
              + "\n";
    cases.push_back(&c);
  }
  source += "}\n";
  xsmp::SmpWorld world;
  const auto document = world.parse(".xsmpcfg", source);
  ASSERT_TRUE(document->parseSucceeded());
  const auto exported = xsmp::exportDocument(*document);
  pugi::xml_document xml;
  ASSERT_TRUE(xml.load_string(exported.c_str()));
  std::size_t index = 0;
  for (const auto item : xml.document_element().child("Component").children()) {
    SCOPED_TRACE(cases[index]->source);
    const std::string_view written = item.attribute("Value").value();
    EXPECT_TRUE(xmlCanHold(written));
    EXPECT_EQ(xsmp::decodeXmlText(written), cases[index]->value);
    ++index;
  }
  EXPECT_EQ(index, cases.size());
  xsmp::ScratchDirectory dir;
  const auto imported =
      xsmp::renderImportedDocument(dir.write("input.smpcfg", exported));
  EXPECT_TRUE(imported.warnings.empty());
  for (std::size_t i = 0; i < cases.size(); ++i) {
    const auto line = "k" + std::to_string(i) + " = "
                      + std::string(cases[i]->imported) + "\n";
    EXPECT_NE(imported.content.find(line), std::string::npos)
        << line << imported.content;
  }
  xsmp::SmpWorld again;
  EXPECT_EQ(xsmp::exportDocument(*again.parse(".xsmpcfg", imported.content)),
            exported);
}

// Every literal of the table in each place of a catalogue, in each standard.
TEST(SmpRoundtrip, EveryEscapeOfACatalogueComesBack) {
  std::string source = R"(catalogue N namespace n {
  /** @uuid 11111111-1111-1111-1111-111111111111 */
  public array Texts = Smp.String8[1]
  /** @uuid 11111111-1111-1111-1111-111111111112 */
  public array Chars = Smp.Char8[1]
  /** @uuid 11111111-1111-1111-1111-111111111113 */
  public struct H { field Smp.String8 s field Smp.Char8 c }
  /** @uuid 11111111-1111-1111-1111-111111111114 */
  public attribute Smp.String8 Text = ""
  /** @uuid 11111111-1111-1111-1111-111111111115 */
  public attribute Smp.Char8 Letter = 'x'
  /** @uuid 11111111-1111-1111-1111-111111111116 */
  public struct S {
)";
  std::string parameters;
  const auto& cases = xsmp::escapes::cases();
  for (std::size_t i = 0; i < cases.size(); ++i) {
    const auto& c = cases[i];
    const auto n = std::to_string(i);
    const auto literal = std::string(c.source);
    const auto type = c.isChar() ? "Smp.Char8 " : "Smp.String8 ";
    source +=
        std::string("constant ") + type + "k" + n + " = " + literal + "\n";
    source += std::string(c.isChar() ? "@n.Letter(" : "@n.Text(") + literal
              + ") field " + type + "f" + n + " = " + literal + "\n";
    source += std::string("field n.") + (c.isChar() ? "Chars" : "Texts") + " a"
              + n + " = {" + literal + "}\n";
    source += "field n.H h" + n + " = {"
              + (c.isChar() ? ".s = \"\", .c = " + literal : ".s = " + literal)
              + "}\n";
    parameters +=
        (i ? ", in " : "in ") + std::string(type) + "p" + n + " = " + literal;
  }
  source +=
      "}\n/** @uuid 11111111-1111-1111-1111-111111111117 */\n"
      "public interface I { def void f("
      + parameters + ") } }\n";

  for (const auto* standard : {"ECSS_SMP_2020", "ECSS_SMP_2025"}) {
    SCOPED_TRACE(standard);
    xsmp::SmpProject project(standard);
    const auto document = project.add("n.xsmpcat", source);
    project.build();
    ASSERT_TRUE(document->parseSucceeded());
    const auto exported = xsmp::exportDocument(*document);
    pugi::xml_document xml;
    ASSERT_TRUE(xml.load_string(exported.c_str()));
    const auto has2025Defaults = std::string_view(standard) == "ECSS_SMP_2025";
    // One value of the XML against the table.
    const auto holds = [&](std::size_t i, std::string_view what,
                           pugi::xml_node node) {
      SCOPED_TRACE(what);
      ASSERT_TRUE(node);
      const std::string_view written = node.attribute("Value").value();
      EXPECT_TRUE(xmlCanHold(written));
      EXPECT_EQ(xsmp::decodeXmlText(written), cases[i].value);
    };
    const auto types = xml.document_element().child("Namespace");
    const auto structure = types.find_child_by_attribute("Id", "n.S");
    const auto operation =
        types.find_child_by_attribute("Id", "n.I").child("Operation");
    ASSERT_TRUE(structure && operation);
    for (std::size_t i = 0; i < cases.size(); ++i) {
      SCOPED_TRACE(cases[i].source);
      const auto n = std::to_string(i);
      const auto member = [&](const std::string& name) {
        return structure.find_child_by_attribute("Name", name.c_str());
      };
      holds(i, "constant", member("k" + n).child("Value"));
      holds(i, "field", member("f" + n).child("Default"));
      holds(i, "attribute", member("f" + n).child("Metadata").child("Value"));
      holds(i, "array", member("a" + n).child("Default").child("ItemValue"));
      holds(i, "structure",
            member("h" + n).child("Default").find_child_by_attribute(
                "Field", cases[i].isChar() ? "c" : "s"));
      const auto parameter =
          operation.find_child_by_attribute("Name", ("p" + n).c_str());
      if (has2025Defaults) {
        // In ECSS SMP 2025 a default is plain C++ text.
        const std::string_view written = parameter.child_value("Default");
        EXPECT_TRUE(xmlCanHold(written));
        // A default is not C++ of a standard: no trigraph to keep from reading.
        auto cpp = std::string(cases[i].cpp);
        if (const auto at = cpp.find(R"(?\?)"); at != std::string::npos)
          cpp.erase(at + 1, 1);
        EXPECT_EQ(written, cpp);
      } else {
        holds(i, "parameter", parameter.child("Default"));
      }
    }

    xsmp::ScratchDirectory dir;
    const auto imported =
        xsmp::renderImportedDocument(dir.write("input.smpcat", exported));
    EXPECT_TRUE(imported.warnings.empty());
    for (std::size_t i = 0; i < cases.size(); ++i) {
      SCOPED_TRACE(cases[i].source);
      const auto n = std::to_string(i);
      const auto spelling = std::string(cases[i].imported);
      for (const auto& expected : {" k" + n + " = " + spelling + "\n",
                                   " f" + n + " = " + spelling + "\n",
                                   " a" + n + " = {" + spelling + "}\n"})
        EXPECT_NE(imported.content.find(expected), std::string::npos)
            << expected << imported.content;
    }
    xsmp::SmpProject again(standard);
    const auto back = again.add("n.xsmpcat", imported.content);
    again.build();
    EXPECT_EQ(xsmp::exportDocument(*back), exported);
  }
}

// A golden of each kind of document, imported, is exported again as it was.
TEST(SmpRoundtrip, TheReferenceGoldensComeBackUnchanged) {
  const auto fixtures =
      std::filesystem::path(XSMP_FIXTURES_DIR) / "generator/smp";
  for (const auto& [standard, golden] :
       {std::pair{"ECSS_SMP_2020", "reference-catalogue/expected-2020"},
        std::pair{"ECSS_SMP_2025", "reference-configuration/expected-2025"},
        std::pair{"ECSS_SMP_2025", "reference-assembly/expected-2025"},
        std::pair{"ECSS_SMP_2025", "reference-links/expected-2025"},
        std::pair{"ECSS_SMP_2025", "reference-schedule/expected-2025"}}) {
    const auto folder = fixtures / golden / "smdl-gen";
    for (const auto& entry : std::filesystem::directory_iterator(folder)) {
      const auto file = entry.path();
      if (file.stem() != "test" || file.extension() == ".smppkg") continue;
      SCOPED_TRACE(file.string());
      const auto imported = xsmp::renderImportedDocument(file);
      xsmp::SmpProject project(standard);
      const auto extension = ".x" + file.extension().string().substr(1);
      const auto document = project.add("test" + extension, imported.content);
      project.build();
      EXPECT_EQ(xsmp::exportDocument(*document), xsmp::readFile(file));
    }
  }
}
