#include <gtest/gtest.h>

#include <cursiv/workspace/Document.hpp>
#include <cursiv/workspace/Documents.hpp>
#include <memory>
#include <string>
#include <vector>
#include <xsmp/SharedCoreServices.hpp>
#include <xsmp/XsmpprojectCoreModule.hpp>
#include <xsmp/ast.hpp>

#include "support/DiagnosticTestSupport.hpp"

namespace {

using Doc = std::shared_ptr<cursiv::Document>;

// Builds and links all `documents` together (index, then link, no validation).
// Cross-document references resolve after this returns.
void buildAndLink(cursiv::SharedCoreServices& shared,
                  const std::vector<Doc>& documents) {
  cursiv::BuildOptions options;
  options.validation = false;
  xsmp::buildDocuments(*shared.documentBuilder, documents, options);
}

Doc makeDoc(cursiv::SharedCoreServices& shared, std::string uri,
            std::string text) {
  return shared.documents->createDocument(std::move(uri), std::move(text));
}

}  // namespace

// A `dependency` reference resolves to a Project declared in another
// `xsmp.project` file, through the global index.
TEST(XsmpprojectScopeComputation, DependencyResolvesAcrossDocuments) {
  auto shared = xsmp::createXsmpSharedServices();
  auto services = xsmp::createXsmpprojectServices(*shared);
  shared->serviceRegistry->registerServices(std::move(services));

  auto projectA = makeDoc(*shared, "file:///a/xsmp.project", "project 'A'\n");
  auto projectB = makeDoc(*shared, "file:///b/xsmp.project",
                          "project 'B'\ndependency 'A'\n");
  buildAndLink(*shared, {projectA, projectB});

  const auto* b =
      cursiv::ast_cast<const xsmp::Project>(projectB->parseResult.value);
  ASSERT_NE(b, nullptr);
  ASSERT_EQ(b->elements.size(), 1U);
  const auto* dependency =
      cursiv::ast_cast<const xsmp::Dependency>(b->elements[0]);
  ASSERT_NE(dependency, nullptr);
  const auto* target = dependency->project.get();
  ASSERT_NE(target, nullptr);
  EXPECT_EQ(target->name, "A");
}

// A `profile` reference resolves to a Profile declared in a `.xsmpprofile`
// file.
TEST(XsmpprojectScopeComputation,
     ProfileReferenceResolvesToProfileDeclaration) {
  auto shared = xsmp::createXsmpSharedServices();
  auto services = xsmp::createXsmpprojectServices(*shared);
  shared->serviceRegistry->registerServices(std::move(services));

  auto profile =
      makeDoc(*shared, "file:///p/sdk.xsmpprofile", "profile 'xsmp-sdk'\n");
  auto project = makeDoc(*shared, "file:///p/xsmp.project",
                         "project 'P'\nprofile 'xsmp-sdk'\n");
  buildAndLink(*shared, {profile, project});

  const auto* p =
      cursiv::ast_cast<const xsmp::Project>(project->parseResult.value);
  ASSERT_NE(p, nullptr);
  ASSERT_EQ(p->elements.size(), 1U);
  const auto* reference =
      cursiv::ast_cast<const xsmp::ProfileReference>(p->elements[0]);
  ASSERT_NE(reference, nullptr);
  const auto* target = reference->profile.get();
  ASSERT_NE(target, nullptr);
  EXPECT_EQ(target->name, "xsmp-sdk");
}

TEST(XsmpprojectScopeComputation,
     DeprecatedTasMdkAliasResolvesToCanonicalProfileNodeAndWarns) {
  auto shared = xsmp::createXsmpSharedServices();
  shared->serviceRegistry->registerServices(
      xsmp::createXsmpprojectServices(*shared));
  shared->contributionAliases["org.eclipse.xsmp.profile.tas-mdk"] = "tas-mdk";
  shared->contributionDeprecatedAliases.insert(
      "org.eclipse.xsmp.profile.tas-mdk");
  auto profile = makeDoc(*shared, "xsmp:/contributions/tas-mdk.xsmpprofile",
                         "profile 'tas-mdk'\n");
  auto project =
      makeDoc(*shared, "file:///alias/xsmp.project",
              "project 'Alias'\nprofile 'org.eclipse.xsmp.profile.tas-mdk'\n");
  xsmp::buildDocuments(*shared->documentBuilder,
                       std::vector<Doc>{profile, project});
  const auto* root =
      cursiv::ast_cast<xsmp::Project>(project->parseResult.value);
  ASSERT_NE(root, nullptr);
  const auto* reference =
      cursiv::ast_cast<xsmp::ProfileReference>(root->elements[0]);
  ASSERT_NE(reference, nullptr);
  ASSERT_EQ(reference->profile.get(),
            cursiv::ast_cast<xsmp::Profile>(profile->parseResult.value));
  EXPECT_EQ(reference->profile.get()->name, "tas-mdk");
  auto warning = false;
  for (const auto& diagnostic : project->diagnostics) {
    EXPECT_NE(diagnostic.severity, cursiv::DiagnosticSeverity::Error)
        << diagnostic.message;
    if (diagnostic.message
        == "Deprecated: Use the \"tas-mdk\" profile instead.") {
      warning = true;
      EXPECT_EQ(diagnostic.severity, cursiv::DiagnosticSeverity::Warning);
      EXPECT_NE(
          std::ranges::find(diagnostic.tags, cursiv::DiagnosticTag::Deprecated),
          diagnostic.tags.end());
    }
  }
  EXPECT_TRUE(warning);
}

TEST(XsmpprojectScopeComputation,
     ExternalToolAliasesShareCanonicalNodesAndOnlyLegacyAliasesWarn) {
  auto shared = xsmp::createXsmpSharedServices();
  shared->contributionAliases["fake-tool-alias"] = "fake-tool";
  shared->contributionAliases["fake-tool-legacy"] = "fake-tool";
  shared->contributionDeprecatedAliases.insert("fake-tool-legacy");
  shared->serviceRegistry->registerServices(
      xsmp::createXsmpprojectServices(*shared));
  auto tool = makeDoc(*shared, "xsmp:/contributions/fake-tool.xsmptool",
                      "tool 'fake-tool'");
  auto project = makeDoc(
      *shared, "file:///aliases/xsmp.project",
      "project 'Aliases' tool 'fake-tool-alias' tool 'fake-tool-legacy'");
  xsmp::buildDocuments(*shared->documentBuilder,
                       std::vector<Doc>{tool, project});
  const auto* root =
      cursiv::ast_cast<xsmp::Project>(project->parseResult.value);
  ASSERT_NE(root, nullptr);
  ASSERT_EQ(root->elements.size(), 2u);
  const auto* first = cursiv::ast_cast<xsmp::ToolReference>(root->elements[0]);
  const auto* legacy = cursiv::ast_cast<xsmp::ToolReference>(root->elements[1]);
  ASSERT_NE(first, nullptr);
  ASSERT_NE(legacy, nullptr);
  EXPECT_EQ(first->tool.get(), legacy->tool.get());
  EXPECT_EQ(first->tool.get(),
            cursiv::ast_cast<xsmp::Tool>(tool->parseResult.value));
  EXPECT_EQ(std::ranges::count_if(project->diagnostics,
                                  [](const auto& d) {
                                    return d.message ==
                         "Deprecated: Use the \"fake-tool\" tool instead.";
                                  }),
            1);
  EXPECT_TRUE(std::ranges::any_of(project->diagnostics, [](const auto& d) {
    return d.message == "Duplicated tool 'fake-tool'.";
  }));
}
