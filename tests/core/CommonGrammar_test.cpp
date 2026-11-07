#include <gtest/gtest.h>

#include <cstdint>
#include <map>
#include <xsmp/Builtins.hpp>
#include <xsmp/CliWorkspace.hpp>
#include <xsmp/solver/Solver.hpp>
#include <xsmp/utils/XsmpDocumentation.hpp>

#include "support/DiagnosticTestSupport.hpp"

// Every Unicode blank separates tokens, in every language.
TEST(CommonGrammar, AnyBlankSeparatesTokensInEveryLanguage) {
  const auto shared = xsmp::createXsmpSharedServices();
  xsmp::registerLanguages(*shared);
  const std::pair<const char*, const char*> sources[] = {
      {"catalogue.xsmpcat", "catalogue Demo namespace n {}"},
      {"xsmp.project", "project 'Demo' using 'ECSS_SMP_2025'"},
      {"assembly.xsmpasb", "assembly Demo Root: \"External\" {}"},
      {"configuration.xsmpcfg", "configuration Demo"},
      {"links.xsmplnk", "link Demo"},
      {"schedule.xsmpsed", "schedule Demo"}};
  unsigned sequence = 0;
  const auto parse = [&](std::string_view file, std::string_view source,
                         std::string_view space) {
    auto text = std::string(source);
    text.replace(text.find(' '), 1, space);
    const auto document = shared->documents->createDocument(
        "file:///whitespace/" + std::to_string(sequence++) + "/"
            + std::string(file),
        text);
    cursiv::Document* input[]{document.get()};
    shared->documentBuilder->build(input, {.validation = false});
    return document->parseSucceeded()
           && document->parseResult.diagnostics.empty();
  };
  for (const auto& [file, source] : sources) {
    for (const auto* space :
         {" ", "\t", "\n", "\u00a0", "\u2003", "\u2028", "\u3000", "\ufeff"})
      EXPECT_TRUE(parse(file, source, space)) << file << ": " << space;
    // A byte order mark at the start of a file, as some editors write it.
    EXPECT_TRUE(parse(file, "\ufeff" + std::string(source), " ")) << file;
  }
}

// Documents with Windows line endings parse in every language.
TEST(CommonGrammar, CarriageReturnLineFeedIsABlankInEveryLanguage) {
  const auto shared = xsmp::createXsmpSharedServices();
  xsmp::registerLanguages(*shared);
  const std::pair<const char*, const char*> sources[] = {
      {"catalogue.xsmpcat", "catalogue Demo\r\nnamespace n {\r\n}\r\n"},
      {"xsmp.project", "project 'Demo'\r\nsource 'src'\r\n"},
      {"assembly.xsmpasb", "assembly Demo\r\nRoot: \"External\" {}\r\n"},
      {"configuration.xsmpcfg", "configuration Demo\r\n/root {}\r\n"},
      {"links.xsmplnk", "link Demo\r\n"},
      {"schedule.xsmpsed", "schedule Demo\r\n"}};
  unsigned sequence = 0;
  for (const auto& [file, source] : sources) {
    const auto document = shared->documents->createDocument(
        "file:///crlf/" + std::to_string(sequence++) + "/" + file, source);
    cursiv::Document* input[]{document.get()};
    shared->documentBuilder->build(input, {.validation = false});
    EXPECT_TRUE(document->parseSucceeded()
                && document->parseResult.diagnostics.empty())
        << file;
  }
}
