#include <gtest/gtest.h>
#include <lsp/types.h>

#include <cursiv/navigation/RenameProvider.hpp>
#include <string>
#include <string_view>

#include "support/LspTestSupport.hpp"

namespace {

// Renames the declaration whose name first occurs at `symbol` in `text`, then
// returns the single edited document's resulting text.
std::string renameFirst(xsmp::TestWorkspace& ws, const std::string& text,
                        std::string_view symbol, std::string_view newName) {
  auto result = cursiv::test::validationHelper(ws.language("xsmpcat"), text);
  if (result.document == nullptr) {
    return "<no document>";
  }
  const auto position =
      cursiv::to_lsp(result.document->textDocument().positionAt(
          static_cast<cursiv::TextOffset>(text.find(symbol))));
  const auto edit = ws.language("xsmpcat").renameProvider->rename(
      *result.document, position, newName);
  if (!edit.has_value() || !edit->changes.has_value()
      || edit->changes->empty()) {
    return "<no edit>";
  }
  return xsmp::applyTextEdits(*result.document, edit->changes->begin()->second);
}

}  // namespace

// Renaming a Type rewrites both its declaration and its (simple) references.
TEST(XsmpRenameProvider, RenamesDeclarationAndSimpleReference) {
  auto ws = xsmp::xsmpcatWorkspace();
  EXPECT_EQ(renameFirst(ws,
                        "catalogue c\n"
                        "namespace n\n"
                        "{\n"
                        "  primitive Bool\n"
                        "  struct S { field Bool x }\n"
                        "}\n",
                        "Bool", "Boolean"),
            "catalogue c\n"
            "namespace n\n"
            "{\n"
            "  primitive Boolean\n"
            "  struct S { field Boolean x }\n"
            "}\n");
}

// A reference written with a qualifier keeps the qualifier, with only the
// renamed segment rewritten (Bool -> Boolean, `n.` preserved).
TEST(XsmpRenameProvider, PreservesQualifierOnQualifiedReference) {
  auto ws = xsmp::xsmpcatWorkspace();
  EXPECT_EQ(renameFirst(ws,
                        "catalogue c\n"
                        "namespace n { primitive Bool }\n"
                        "namespace m { struct S { field n.Bool x } }\n",
                        "Bool", "Boolean"),
            "catalogue c\n"
            "namespace n { primitive Boolean }\n"
            "namespace m { struct S { field n.Boolean x } }\n");
}

TEST(XsmpRenameProvider, PreservesPartiallyQualifiedReferencesAndTheirScope) {
  auto ws = xsmp::xsmpcatWorkspace();
  EXPECT_EQ(renameFirst(ws,
                        "catalogue c\n"
                        "namespace a { namespace b { primitive Bool }\n"
                        "  struct S { field b.Bool x } }\n",
                        "Bool", "Boolean"),
            "catalogue c\n"
            "namespace a { namespace b { primitive Boolean }\n"
            "  struct S { field b.Boolean x } }\n");
}

TEST(XsmpRenameProvider,
     RenamingNamespaceLeavesLocalDescendantReferencesLocal) {
  auto ws = xsmp::xsmpcatWorkspace();
  EXPECT_EQ(renameFirst(ws,
                        "catalogue c\n"
                        "namespace outer { namespace inner { primitive Bool }\n"
                        "  struct S { field inner.Bool x } }\n",
                        "outer", "renamed"),
            "catalogue c\n"
            "namespace renamed { namespace inner { primitive Bool }\n"
            "  struct S { field inner.Bool x } }\n");
}

// The placeholders of a template parameter are references of it. The default
// rename reads each reference back as a whole name, which a placeholder is not:
// it refuses, rather than leave them on the old name.
TEST(XsmpRenameProvider, TemplateParameterWithPlaceholdersIsNotRenamedHalfway) {
  auto world = xsmp::deploymentWorkspace();
  const std::string text =
      "assembly <Variant = \"Ops\"> A\nRoot: n.M\n{\n"
      "    kids += System{Variant}: n.M\n}\n";
  const auto document =
      xsmp::parse(world, "xsmpasb", text, "file:///r/a.xsmpasb");
  const auto position = cursiv::to_lsp(document->textDocument().positionAt(
      static_cast<cursiv::TextOffset>(text.find("Variant"))));
  EXPECT_FALSE(world.language("xsmpasb").renameProvider->rename(
      *document, position, "Flavor"));
}
