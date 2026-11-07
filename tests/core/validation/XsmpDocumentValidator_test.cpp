#include <gtest/gtest.h>

#include <algorithm>
#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include <xsmp/Builtins.hpp>
#include <xsmp/CliWorkspace.hpp>
#include <xsmp/solver/Solver.hpp>
#include <xsmp/utils/XsmpDocumentation.hpp>

#include "support/DiagnosticTestSupport.hpp"

namespace {

auto workspace() {
  auto result = xsmp::TestWorkspace::withShared<xsmp::XsmpSharedServices>();
  auto& shared = result.shared();
  shared.projectManager = std::make_unique<xsmp::ProjectManager>(shared);
  result.registerLanguage(xsmp::createXsmpcatServices(shared));
  xsmp::loadXsmpcatBuiltins(shared);
  return result;
}

}  // namespace

TEST(XsmpDocumentValidator, DiagnosticDataOfMissingUuidAndLinkingError) {
  auto ws = workspace();
  const auto result = cursiv::test::validationHelper(
      ws.language("xsmpcat"),
      "catalogue Metadata namespace n { /** é😀 */ public "
      "struct Root { field Missing absent } }");
  ASSERT_TRUE(result.document->parseSucceeded());
  auto missing = false, linking = false;
  for (const auto& issue : result.diagnostics) {
    if (issue.message == "Missing Type UUID.") {
      missing = true;
      ASSERT_TRUE(issue.data);
      EXPECT_EQ(issue.data->toJsonString({.pretty = false}),
                "{\"code\":\"missing-uuid\"}");
    }
    if (issue.message
        == "Could not resolve reference to Type named 'Missing'.") {
      linking = true;
      ASSERT_TRUE(issue.data);
      EXPECT_EQ(issue.data->toJsonString({.pretty = false}),
                "{\"code\":\"linking-error\",\"containerType\":\"Field\","
                "\"property\":\"type\",\"refText\":\"Missing\"}");
    }
  }
  EXPECT_TRUE(missing);
  EXPECT_TRUE(linking);
}

// The data of a linking error names the type that declares the reference and
// the member with its suffix, if it has one.
TEST(XsmpDocumentValidator, LinkingErrorDataNamesTheDeclaringTypeAndTheMember) {
  auto ws = workspace();
  const auto result = cursiv::test::validationHelper(
      ws.language("xsmpcat"),
      "catalogue Links namespace n { public model M extends Nope implements "
      "Nada { reference Vanished r } }");
  std::map<std::string, std::string> data;
  for (const auto& issue : result.diagnostics)
    if (issue.data && cursiv::data_code(issue) == "linking-error")
      data[issue.data->object().at("refText").string()] =
          issue.data->toJsonString({.pretty = false});
  EXPECT_EQ(data["Nope"],
            "{\"code\":\"linking-error\",\"containerType\":\"Component\","
            "\"property\":\"base\",\"refText\":\"Nope\"}");
  EXPECT_EQ(data["Nada"],
            "{\"code\":\"linking-error\",\"containerType\":\"Component\","
            "\"property\":\"interface_\",\"refText\":\"Nada\"}");
  EXPECT_EQ(data["Vanished"],
            "{\"code\":\"linking-error\",\"containerType\":\"Reference\","
            "\"property\":\"interface_\",\"refText\":\"Vanished\"}");
}

namespace {

// A project of two files with one name, in two folders.
struct SameFileName {
  std::unique_ptr<xsmp::XsmpSharedCoreServices> shared =
      xsmp::createXsmpSharedServices();

  SameFileName() { xsmp::registerLanguages(*shared); }

  std::vector<std::shared_ptr<cursiv::Document>> build(
      std::string_view project, std::string_view extension) {
    const auto text = [&](std::string_view name) {
      return std::string(extension == "xsmpcat" ? "catalogue "
                                                : "configuration ")
             + std::string(name) + "\n";
    };
    const auto file = "x." + std::string(extension);
    const std::vector documents{
        shared->documents->createDocument("file:///p/xsmp.project",
                                          std::string(project)),
        shared->documents->createDocument("file:///p/smdl/a/" + file,
                                          text("A")),
        shared->documents->createDocument("file:///p/smdl/b/" + file,
                                          text("B")),
    };
    std::vector<cursiv::Document*> raw;
    for (const auto& document : documents) raw.push_back(document.get());
    shared->documentBuilder->build(raw);
    return documents;
  }

  static bool refused(const cursiv::Document& document) {
    return std::ranges::any_of(document.diagnostics, [](const auto& d) {
      return d.message.find("has this file name") != std::string::npos;
    });
  }
};

}  // namespace

TEST(XsmpDocumentValidator, TwoDocumentsOfAProjectThatWriteOneFileAreRefused) {
  for (const auto* extension : {"xsmpcfg", "xsmpcat"}) {
    SameFileName world;
    const auto documents =
        world.build("project 'P'\nsource 'smdl'\ntool 'smp'\n", extension);
    EXPECT_TRUE(SameFileName::refused(*documents[1])) << extension;
    EXPECT_TRUE(SameFileName::refused(*documents[2])) << extension;
    const auto found = std::ranges::find_if(
        documents[1]->diagnostics,
        [](const auto& d) { return !d.relatedInformation.empty(); });
    ASSERT_NE(found, documents[1]->diagnostics.end());
    EXPECT_EQ(found->relatedInformation[0].uri, documents[2]->uri);
  }
}

// Only the tool `smp` writes a document under its file name.
TEST(XsmpDocumentValidator, WithoutTheSmpToolTwoFilesMayHaveOneName) {
  SameFileName world;
  const auto documents = world.build("project 'P'\nsource 'smdl'\n", "xsmpcfg");
  EXPECT_FALSE(SameFileName::refused(*documents[1]));
  EXPECT_FALSE(SameFileName::refused(*documents[2]));
}

TEST(XsmpDocumentValidator, TheDocumentsOfAnotherProjectWriteElsewhere) {
  SameFileName world;
  const auto documents =
      world.build("project 'P'\nsource 'smdl/a'\ntool 'smp'\n", "xsmpcfg");
  EXPECT_FALSE(SameFileName::refused(*documents[1]));
  auto other = world.shared->documents->createDocument(
      "file:///q/xsmp.project", "project 'Q'\nsource 'smdl'\ntool 'smp'\n");
  auto twin = world.shared->documents->createDocument(
      "file:///q/smdl/x.xsmpcfg", "configuration Q\n");
  std::vector<cursiv::Document*> raw{other.get(), twin.get()};
  world.shared->documentBuilder->build(raw);
  EXPECT_FALSE(SameFileName::refused(*twin));
}

// Bytes that are not UTF-8: a warning once per file in a comment, an error on a
// literal, and nothing at all for valid text.
TEST(XsmpDocumentValidator, BytesThatAreNotUtf8DependOnWhereTheyStand) {
  struct Found {
    std::vector<std::uint32_t> warnings;
    std::vector<std::uint32_t> errors;
  };

  constexpr std::string_view warning = "This file is not valid UTF-8.";
  constexpr std::string_view error =
      R"(This literal is not valid UTF-8: use an escape such as \xE9.)";
  const auto check = [&](std::string_view name, std::string_view prefix,
                         std::string_view bytes, std::string_view suffix) {
    const auto shared = xsmp::createXsmpSharedServices();
    xsmp::registerLanguages(*shared);
    auto document = shared->documents->createDocument(
        "file:///utf8/" + std::string(name),
        std::string(prefix) + std::string(bytes) + std::string(suffix));
    std::vector<cursiv::Document*> raw{document.get()};
    shared->documentBuilder->build(raw);
    Found found;
    for (const auto& diagnostic : document->diagnostics)
      if (diagnostic.message == warning) {
        EXPECT_EQ(diagnostic.severity, cursiv::DiagnosticSeverity::Warning);
        found.warnings.push_back(diagnostic.range.begin);
      } else if (diagnostic.message == error) {
        EXPECT_EQ(diagnostic.severity, cursiv::DiagnosticSeverity::Error);
        found.errors.push_back(diagnostic.range.begin);
      }
    return found;
  };
  constexpr std::string_view comments[][3] = {
      {"c.xsmpcat", "catalogue c /* ", " */ namespace n {}\n"},
      {"c.xsmpcat", "/** ", " */ catalogue c namespace n {}\n"},
      {"c.xsmpcat", "catalogue c // ", "\nnamespace n {}\n"},
      {"d.xsmpcfg", "configuration C // ", "\n/Root { a = 1i32 }\n"},
      {"p.xsmpsed", "/* ", " */ schedule <R = \"r\"> P\n"},
  };
  constexpr std::string_view literals[][3] = {
      {"c.xsmpcat",
       "catalogue c namespace n { struct S { constant Smp.String8 k = \"",
       "\" } }\n"},
      {"d.xsmpcfg", "configuration C /Root { a = \"", "\" }\n"},
      {"p.xsmpsed", "schedule <R = \"", "\"> P\n"},
  };
  constexpr std::string_view valid[] = {"é",
                                        "€",
                                        "😀",
                                        "\xED\x9F\xBF",
                                        "\xEE\x80\x80",
                                        "\xEF\xBF\xBE",
                                        "\xEF\xBF\xBF",
                                        "\xF4\x8F\xBF\xBF",
                                        "\xE0\xA0\x80",
                                        "\xF0\x90\x80\x80",
                                        "\xC2\x80"};
  constexpr std::string_view invalid[] = {"\xFF",
                                          "\xFE",
                                          "\x80",
                                          "\xBF",
                                          "\xC3",
                                          "\xC0\x80",
                                          "\xC1\xBF",
                                          "\xE2\x82",
                                          "\xE0\x80\x80",
                                          "\xE0\x9F\xBF",
                                          "\xED\xA0\x80",
                                          "\xED\xBF\xBF",
                                          "\xF0\x80\x80\x80",
                                          "\xF0\x8F\xBF\xBF",
                                          "\xF4\x90\x80\x80",
                                          "\xF5\x80\x80\x80",
                                          "\xF8\x88\x80\x80\x80",
                                          "\xC3(",
                                          "\xE2\x82(",
                                          "\xF0\x9F\x98("};
  for (const auto& [name, prefix, suffix] : comments) {
    SCOPED_TRACE(prefix);
    for (const auto bytes : valid) {
      const auto found = check(name, prefix, bytes, suffix);
      EXPECT_TRUE(found.warnings.empty() && found.errors.empty()) << bytes;
    }
    for (const auto bytes : invalid) {
      const auto found = check(name, prefix, bytes, suffix);
      EXPECT_EQ(
          found.warnings,
          std::vector<std::uint32_t>{static_cast<std::uint32_t>(prefix.size())})
          << bytes;
      EXPECT_TRUE(found.errors.empty()) << bytes;
    }
    EXPECT_EQ(check(name, prefix, "ok\xFF\xFF", suffix).warnings,
              std::vector<std::uint32_t>{
                  static_cast<std::uint32_t>(prefix.size() + 2)});
  }
  for (const auto& [name, prefix, suffix] : literals) {
    SCOPED_TRACE(prefix);
    const auto opening = static_cast<std::uint32_t>(prefix.size() - 1);
    for (const auto bytes : valid) {
      const auto found = check(name, prefix, bytes, suffix);
      EXPECT_TRUE(found.warnings.empty() && found.errors.empty()) << bytes;
    }
    for (const auto bytes : invalid) {
      const auto found = check(name, prefix, bytes, suffix);
      EXPECT_EQ(found.errors, std::vector<std::uint32_t>{opening}) << bytes;
      EXPECT_TRUE(found.warnings.empty()) << bytes;
    }
  }

  // Two comments and two literals: one warning for the file, and each literal.
  const auto found = check(
      "c.xsmpcat",
      "// a\xE9\n/* b\xE9 */ catalogue c namespace n { struct S {\n"
      "constant Smp.String8 k = \"c\xE9\"\nconstant Smp.Char8 l = '\xE9'\n"
      "} }\n",
      "", "");
  EXPECT_EQ(found.warnings, std::vector<std::uint32_t>{4});
  EXPECT_EQ(found.errors.size(), 2U);
}

// A message that quotes a comment holds no such byte.
TEST(XsmpDocumentValidator, AMessageWritesTheBytesOfACommentWithAnEscape) {
  auto ws = workspace();
  const auto result = cursiv::test::validationHelper(
      ws.language("xsmpcat"),
      "catalogue c namespace n {\n"
      "  /** @deprecated vieux \xE9\n"
      "   * @uuid 11111111-1111-1111-1111-111111111111 */\n"
      "  public struct S { }\n"
      "  /** @uuid 11111111-1111-1111-1111-111111111112 */\n"
      "  public struct T { field S s }\n"
      "}\n");
  const auto& diagnostics = result.document->diagnostics;
  EXPECT_NE(std::ranges::find_if(diagnostics,
                                 [](const auto& diagnostic) {
                                   return diagnostic.message
                                          == R"(Deprecated: vieux \xE9)";
                                 }),
            diagnostics.end());
}

// A byte that an escape writes is no concern of the rule on the file.
TEST(XsmpDocumentValidator, AnEscapedByteIsValidText) {
  auto ws = workspace();
  const auto result = cursiv::test::validationHelper(
      ws.language("xsmpcat"), R"(catalogue c namespace n {
    /** @uuid 11111111-1111-1111-1111-111111111111 */
    public struct S {
      constant Smp.String8 a = "\xE9\377"
      constant Smp.Char8 b = '\xE9'
    }
  }
)");
  for (const auto& diagnostic : result.document->diagnostics)
    EXPECT_EQ(diagnostic.message.find("UTF-8"), std::string::npos)
        << diagnostic.message;
}

// A constant is refused for itself, whatever the project writes.
TEST(XsmpDocumentValidator, ANullptrConstantIsRefusedWithOrWithoutTheSmpTool) {
  const auto hasError = [](std::string_view project) {
    const auto shared = xsmp::createXsmpSharedServices();
    xsmp::registerLanguages(*shared);
    auto descriptor = shared->documents->createDocument(
        "file:///null/xsmp.project", std::string(project));
    auto catalogue = shared->documents->createDocument(
        "file:///null/smdl/c.xsmpcat",
        "catalogue c\nnamespace n {\n/** @uuid "
        "11111111-1111-4111-8111-111111111111 */\npublic struct S {\n"
        "constant Smp.String8 k = nullptr\n}\n}\n");
    std::vector<cursiv::Document*> raw{descriptor.get(), catalogue.get()};
    shared->documentBuilder->build(raw);
    return std::ranges::any_of(catalogue->diagnostics, [](const auto& d) {
      return d.message == xsmp::nullptrNeedsAPointer;
    });
  };
  EXPECT_TRUE(hasError("project 'P'\nsource 'smdl'\n"));
  EXPECT_TRUE(hasError("project 'P'\nsource 'smdl'\ntool 'smp'\n"));
}
