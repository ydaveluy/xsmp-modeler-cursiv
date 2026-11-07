#include <gtest/gtest.h>

#include <chrono>
#include <filesystem>
#include <string>
#include <xsmp/esa-cdk/EsaCdkGenerator.hpp>
#include <xsmp/xsmp-sdk/XsmpSdkGenerator.hpp>

#include "support/EscapeCases.hpp"
#include "support/Files.hpp"
#include "support/GeneratorTestSupport.hpp"

// Regenerating an unchanged catalogue must not rewrite identical files.
TEST(CppGenerator, RegeneratingUnchangedCatalogueIsIdempotent) {
  xsmp::ToolFixture fixture;
  const auto& catalogue = fixture.parse("catalogue demo\nnamespace n {}\n");
  const xsmp::XsmpSdkGenerator generator(*fixture.language);
  generator.generate(catalogue, fixture.directory);
  const auto header = fixture.directory / "src-gen/demo.h";

  // Ensure a real, observable time delta if the file WERE rewritten.
  std::filesystem::last_write_time(
      header,
      std::filesystem::last_write_time(header) - std::chrono::seconds(5));
  // Read back, as a WASI host keeps whole seconds.
  const auto backdated = std::filesystem::last_write_time(header);

  generator.generate(catalogue, fixture.directory);
  EXPECT_EQ(std::filesystem::last_write_time(header), backdated)
      << "regenerating identical content must not touch the file";
}

// Each literal has its one spelling in C++, in each place a value can stand.
// The table has the compiler read that spelling, which proves what it holds.
TEST(CppGenerator, EveryEscapeHasItsSpellingInEveryPlace) {
  std::string text = R"(catalogue demo namespace n {
    /** @uuid 11111111-1111-1111-1111-111111111111 */
    public array Texts = Smp.String8[1]
    /** @uuid 11111111-1111-1111-1111-111111111112 */
    public array Chars = Smp.Char8[1]
    /** @uuid 11111111-1111-1111-1111-111111111113 */
    public struct H { field Smp.String8 s field Smp.Char8 c }
    /** @uuid 11111111-1111-1111-1111-111111111114 */
    public struct S {
)";
  std::string parameters;
  const auto& cases = xsmp::escapes::cases();
  for (std::size_t i = 0; i < cases.size(); ++i) {
    const auto& c = cases[i];
    const auto n = std::to_string(i);
    const auto source = std::string(c.source);
    const auto type = c.isChar() ? "Smp.Char8 " : "Smp.String8 ";
    text += std::string("constant ") + type + "k" + n + " = " + source + "\n";
    text += std::string("field ") + type + "f" + n + " = " + source + "\n";
    text += std::string("field n.") + (c.isChar() ? "Chars" : "Texts") + " a"
            + n + " = {" + source + "}\n";
    text += "field n.H h" + n + " = {"
            + (c.isChar() ? ".s = \"\", .c = " + source : ".s = " + source)
            + "}\n";
    parameters +=
        (i ? ", in " : "in ") + std::string(type) + "p" + n + " = " + source;
  }
  text +=
      "}\n/** @uuid 11111111-1111-1111-1111-111111111115 */\n"
      "public interface I { def void f("
      + parameters + ") } }\n";
  xsmp::ToolFixture fixture;
  const auto& catalogue = fixture.parse(text);
  const xsmp::ScratchDirectory sdk, cdk;
  xsmp::XsmpSdkGenerator(*fixture.language).generate(catalogue, sdk.path);
  xsmp::EsaCdkGenerator(*fixture.language).generate(catalogue, cdk.path);
  for (const auto* dir : {&sdk, &cdk}) {
    const auto header = xsmp::readFile(dir->path / "src-gen/n/S.h");
    const auto operation = xsmp::readFile(dir->path / "src-gen/n/IGen.h");
    for (std::size_t i = 0; i < cases.size(); ++i) {
      SCOPED_TRACE(cases[i].source);
      const auto& c = cases[i];
      const auto n = std::to_string(i);
      // A trigraph only reads as one up to C++14, and the SDK is on C++17.
      auto spelling = std::string(c.cpp);
      if (const auto at = spelling.find(R"(?\?)");
          dir == &sdk && at != std::string::npos)
        spelling.erase(at + 1, 1);
      for (const auto& expected :
           {" k" + n + "{" + spelling + "};", " f" + n + "{" + spelling + "};",
            " a" + n + "{{ " + spelling + "}};",
            " h" + n + "{ /* .s = */" + (c.isChar() ? "\"\", /* .c = */" : "")
                + spelling + "};"})
        EXPECT_NE(header.find(expected), std::string::npos) << expected;
      const auto parameter = " p" + n + " = " + spelling;
      EXPECT_NE(operation.find(parameter), std::string::npos) << parameter;
    }
  }
}
