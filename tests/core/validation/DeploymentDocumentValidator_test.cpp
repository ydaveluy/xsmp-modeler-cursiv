#include <gtest/gtest.h>

#include <cursiv/workspace/Document.hpp>
#include <cursiv/workspace/DocumentBuilder.hpp>
#include <cursiv/workspace/DocumentFactory.hpp>
#include <cursiv/workspace/Documents.hpp>
#include <memory>
#include <vector>

#include "core/validation/DeploymentSemanticsFixture.hpp"

using DeploymentDocumentValidatorTest = DeploymentSemantics;

TEST_F(DeploymentDocumentValidatorTest,
       DuplicateDocumentNamesAreRejectedAcrossLanguages) {
  auto document =
      build("xsmpcfg", "configuration DemoAsm\n/root: demo.Root {}\n");
  EXPECT_TRUE(has(*document, "Duplicated Document name."));
}

TEST_F(DeploymentDocumentValidatorTest,
       CatalogueAlsoRejectsDuplicateNamesAcrossDocumentKinds) {
  auto catalogue = add("src/duplicate.xsmpcat", "catalogue CrossKind\n");
  auto document = build("xsmpcfg", "configuration CrossKind\n/root {}\n");
  EXPECT_TRUE(has(*catalogue, "Duplicated Document name."));
  EXPECT_TRUE(has(*document, "Duplicated Document name."));
}

TEST_F(DeploymentDocumentValidatorTest,
       RemovingAndRestoringDependencyRelinksEveryDeploymentLanguage) {
  add("lib/xsmp.project", "project 'Lib' source 'src'");
  auto project =
      add("app/xsmp.project", "project 'App' source 'src' dependency 'Lib'");
  add("lib/src/t.xsmpcat",
      "catalogue Library namespace lib { public model Root {} }");
  add("lib/src/a.xsmpasb", "assembly LibraryAssembly Root: lib.Root {}");
  const std::vector<std::shared_ptr<cursiv::Document>> users{
      add("app/src/a.xsmpasb", "assembly Use Root: lib.Root {}"),
      add("app/src/c.xsmpcfg",
          "configuration Config /root: LibraryAssembly {}"),
      add("app/src/l.xsmplnk",
          "link Links for LibraryAssembly / { field link unsafe x -> unsafe y "
          "}"),
      add("app/src/s.xsmpsed",
          "schedule <Root=\"root\"> Tasks task Main on LibraryAssembly { emit "
          "\"Boot\" }")};
  std::vector<cursiv::Document*> raw;
  for (const auto& document : documents) raw.push_back(document.get());
  shared->documentBuilder->build(raw);
  for (const auto& document : users) clean(*document);
  const auto update = [&](std::string text, int version) {
    project->isOpen = true;
    shared->documentFactory->attachTextDocument(
        *project,
        cursiv::TextDocument::create(std::string(project->uri), "xsmpproject",
                                     version, std::move(text)));
    const std::string_view changed[] = {project->uri};
    shared->documentBuilder->update(changed, {});
  };
  update("project 'App' source 'src'", 2);
  for (const auto& document : users)
    EXPECT_TRUE(has(*document, "Could not resolve")) << document->uri;
  update("project 'App' source 'src' dependency 'Lib'", 3);
  for (const auto& document : users) clean(*document);
}
