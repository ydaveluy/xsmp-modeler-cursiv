#include <gtest/gtest.h>

#include <cursiv/parser/Parser.hpp>
#include <cursiv/workspace/Document.hpp>
#include <memory>
#include <string>
#include <xsmp/SharedCoreServices.hpp>
#include <xsmp/XsmpprojectCoreModule.hpp>
#include <xsmp/ast.hpp>

#include "support/DiagnosticTestSupport.hpp"

namespace {

// Builds one `.xsmp.project`/`.xsmpprofile`/`.xsmptool` fixture through the
// core pipeline. Validation is disabled and only the parse result is
// inspected.
std::shared_ptr<cursiv::Document> parseFixture(
    xsmp::XsmpSharedCoreServices& shared, const std::string& relPath) {
  auto services = xsmp::createXsmpprojectServices(shared);
  auto& langServices = *services;
  shared.serviceRegistry->registerServices(std::move(services));
  return xsmp::parsePath(
      std::string(XSMP_FIXTURES_DIR) + "/core/xsmpproject/" + relPath,
      langServices,
      /*validation=*/false);
}

}  // namespace

// The `xsmp.project` file (matched by its file NAME) parses into a Project with
// its standard and four elements (source, dependency, profile, tool).
TEST(XsmpprojectGrammar, ProjectFileParses) {
  auto shared = xsmp::createXsmpSharedServices();
  auto document = parseFixture(*shared, "xsmp.project");
  ASSERT_TRUE(document->parseSucceeded());

  const auto* project =
      cursiv::ast_cast<const xsmp::Project>(document->parseResult.value);
  ASSERT_NE(project, nullptr);
  EXPECT_EQ(project->name, "MissionDemo");
  EXPECT_EQ(project->standard, "ECSS_SMP_2025");
  ASSERT_EQ(project->elements.size(), 4U);
  EXPECT_NE(cursiv::ast_cast<const xsmp::Source>(project->elements[0]),
            nullptr);
  EXPECT_NE(cursiv::ast_cast<const xsmp::Dependency>(project->elements[1]),
            nullptr);
  EXPECT_NE(
      cursiv::ast_cast<const xsmp::ProfileReference>(project->elements[2]),
      nullptr);
  EXPECT_NE(cursiv::ast_cast<const xsmp::ToolReference>(project->elements[3]),
            nullptr);
}

// A `.xsmpprofile` file parses into a Profile declaration.
TEST(XsmpprojectGrammar, ProfileFileParses) {
  auto shared = xsmp::createXsmpSharedServices();
  auto document = parseFixture(*shared, "sample.xsmpprofile");
  ASSERT_TRUE(document->parseSucceeded());
  const auto* profile =
      cursiv::ast_cast<const xsmp::Profile>(document->parseResult.value);
  ASSERT_NE(profile, nullptr);
  EXPECT_EQ(profile->name, "xsmp-sdk");
}

// A `.xsmptool` file parses into a Tool declaration.
TEST(XsmpprojectGrammar, ToolFileParses) {
  auto shared = xsmp::createXsmpSharedServices();
  auto document = parseFixture(*shared, "sample.xsmptool");
  ASSERT_TRUE(document->parseSucceeded());
  const auto* tool =
      cursiv::ast_cast<const xsmp::Tool>(document->parseResult.value);
  ASSERT_NE(tool, nullptr);
  EXPECT_EQ(tool->name, "smp");
}

TEST(XsmpprojectGrammar, CarriageReturnLineFeedIsABlank) {
  auto shared = xsmp::createXsmpSharedServices();
  auto services = xsmp::createXsmpprojectServices(*shared);
  const auto result =
      services->parser->parse("project 'Demo'\r\nsource 'smdl'\r\n");
  ASSERT_TRUE(result.accepted);
  EXPECT_TRUE(result.diagnostics.empty());
}

// A string is the text between its quotes: a backslash is an ordinary
// character, not the start of an escape.
TEST(XsmpprojectGrammar, StringsAreTheTextBetweenTheirQuotes) {
  auto shared = xsmp::createXsmpSharedServices();
  shared->serviceRegistry->registerServices(
      xsmp::createXsmpprojectServices(*shared));
  auto document = shared->documents->createDocument(
      "file:///escape/xsmp.project",
      R"(project "A\u0041\a\0" source "src\\dir" tool "fake\x41")");
  auto* raw = document.get();
  shared->documentBuilder->build({&raw, 1}, {.validation = false});
  ASSERT_TRUE(document->parseSucceeded());
  const auto* project =
      cursiv::ast_cast<xsmp::Project>(document->parseResult.value);
  ASSERT_NE(project, nullptr);
  EXPECT_EQ(project->name, R"(A\u0041\a\0)");
  const auto* source = cursiv::ast_cast<xsmp::Source>(project->elements[0]);
  ASSERT_NE(source, nullptr);
  EXPECT_EQ(source->path, R"(src\\dir)");
  const auto* tool =
      cursiv::ast_cast<xsmp::ToolReference>(project->elements[1]);
  ASSERT_NE(tool, nullptr);
  EXPECT_EQ(tool->tool.refText, R"(fake\x41)");
}
