#include <gtest/gtest.h>

#include <cursiv/services/SharedCoreServices.hpp>
#include <cursiv/workspace/Document.hpp>
#include <cursiv/workspace/DocumentBuilder.hpp>
#include <cursiv/workspace/Documents.hpp>
#include <memory>
#include <span>
#include <string_view>
#include <vector>
#include <xsmp/CliWorkspace.hpp>
#include <xsmp/CoreModule.hpp>
#include <xsmp/SharedCoreServices.hpp>
#include <xsmp/ast.hpp>
#include <xsmp/workspace/SourceUri.hpp>

#include "support/DiagnosticTestSupport.hpp"
#include "support/LspTestSupport.hpp"

// Registering the language auto-loads the embedded ECSS builtins, so a fresh
// workspace resolves `Bool` (the `Smp.*` shortcut) with no manual builtin
// loading.
TEST(Builtins, AutoLoadedOnRegistration) {
  using namespace xsmp;

  auto shared = xsmp::createXsmpSharedServices();
  ASSERT_TRUE(xsmp::registerXsmpcatServices(*shared));

  auto document = shared->documents->createDocument(
      "file:///user.xsmpcat",
      "catalogue test\n"
      "namespace demo { struct S { field Bool c } }\n");

  cursiv::BuildOptions options;
  options.validation = false;
  auto* raw = document.get();
  shared->documentBuilder->build(std::span(&raw, 1), options);

  ASSERT_TRUE(document->parseSucceeded());
  auto* catalogue = cursiv::ast_cast<Catalogue>(document->parseResult.value);
  ASSERT_NE(catalogue, nullptr);
  ASSERT_FALSE(catalogue->elements.empty());

  const auto* demo = catalogue->elements[0];
  const auto* structS = xsmp::findNamed<Structure>(demo->elements, "S");
  ASSERT_NE(structS, nullptr);
  const auto* fieldC = xsmp::findNamed<Field>(structS->elements, "c");
  ASSERT_NE(fieldC, nullptr);
  ASSERT_TRUE(fieldC->type)
      << "Bool should resolve against the auto-loaded builtins";
  EXPECT_EQ(fieldC->type.get()->name, "Bool");
}

// The catalogues of the standard load without a diagnostic of their own.
TEST(Builtins, StandardCataloguesHaveNoDiagnostics) {
  const auto shared = xsmp::createXsmpSharedServices();
  xsmp::registerLanguages(*shared);
  std::vector<cursiv::Document*> builtins;
  for (const auto& document : shared->documents->all())
    if (xsmp::isBuiltinUri(document->uri)) builtins.push_back(document.get());
  ASSERT_FALSE(builtins.empty());
  shared->documentBuilder->build(builtins, {.validation = true});
  for (const auto* document : builtins)
    for (const auto& diagnostic : document->diagnostics)
      ADD_FAILURE() << document->uri << ": " << diagnostic.message;
}
