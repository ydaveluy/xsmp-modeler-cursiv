#include <gtest/gtest.h>
#include <lsp/types.h>

#include <algorithm>
#include <cursiv/hierarchy/TypeHierarchyProvider.hpp>
#include <string>
#include <string_view>

#include "support/LspTestSupport.hpp"

namespace {

bool hasItem(const std::vector<::lsp::TypeHierarchyItem>& items,
             std::string_view name) {
  return std::ranges::any_of(
      items, [&](const auto& item) { return item.name == name; });
}

}  // namespace

// Supertypes of a derived interface include its base; subtypes of the base
// include the derived interface. Entirely provided by xsmpcat (cursiv wires no
// default type-hierarchy provider).
TEST(XsmpcatTypeHierarchyProvider, SupertypesAndSubtypes) {
  auto ws = xsmp::xsmpcatWorkspace();
  const std::string text =
      "catalogue c\n"
      "namespace n\n"
      "{\n"
      "  interface IBase {}\n"
      "  interface IDerived extends IBase {}\n"
      "}\n";
  auto result = cursiv::test::validationHelper(ws.language("xsmpcat"), text);
  ASSERT_NE(result.document, nullptr);
  const auto uri = ::lsp::DocumentUri(::lsp::Uri::parse(result.document->uri));
  const auto positionOf = [&](std::string_view needle) {
    return cursiv::to_lsp(result.document->textDocument().positionAt(
        static_cast<cursiv::TextOffset>(text.find(needle))));
  };

  // Supertypes of IDerived -> IBase.
  ::lsp::TypeHierarchyPrepareParams prepareDerived{};
  prepareDerived.textDocument.uri = uri;
  prepareDerived.position = positionOf("IDerived");
  const auto derivedItems =
      ws.language("xsmpcat").typeHierarchyProvider->prepareTypeHierarchy(
          *result.document, prepareDerived.position);
  ASSERT_EQ(derivedItems.size(), 1U);
  ::lsp::TypeHierarchySupertypesParams supertypesParams{};
  supertypesParams.item = derivedItems.front();
  EXPECT_TRUE(hasItem(ws.language("xsmpcat").typeHierarchyProvider->supertypes(
                          supertypesParams.item),
                      "IBase"));

  // Subtypes of IBase -> IDerived.
  ::lsp::TypeHierarchyPrepareParams prepareBase{};
  prepareBase.textDocument.uri = uri;
  prepareBase.position = positionOf("IBase {");
  const auto baseItems =
      ws.language("xsmpcat").typeHierarchyProvider->prepareTypeHierarchy(
          *result.document, prepareBase.position);
  ASSERT_EQ(baseItems.size(), 1U);
  ::lsp::TypeHierarchySubtypesParams subtypesParams{};
  subtypesParams.item = baseItems.front();
  EXPECT_TRUE(hasItem(ws.language("xsmpcat").typeHierarchyProvider->subtypes(
                          subtypesParams.item),
                      "IDerived"));
}

TEST(XsmpcatTypeHierarchyProvider, NumericBasesKindsDetailsAndDeprecatedTags) {
  auto ws = xsmp::xsmpcatWorkspace();
  const std::string text =
      "catalogue c\nnamespace n {\n"
      " primitive Int32\n primitive Float64\n"
      " /** @deprecated Old numeric type. */\n"
      " integer Count extends Int32\n"
      " float Ratio extends Float64\n}\n";
  const auto result =
      cursiv::test::validationHelper(ws.language("xsmpcat"), text);
  ASSERT_TRUE(result.document->parseSucceeded());
  const auto prepare = [&](std::string_view needle) {
    return ws.language("xsmpcat").typeHierarchyProvider->prepareTypeHierarchy(
        *result.document,
        cursiv::to_lsp(result.document->textDocument().positionAt(
            static_cast<cursiv::TextOffset>(text.find(needle)))));
  };
  const auto count = prepare("Count");
  ASSERT_EQ(count.size(), 1U);
  EXPECT_EQ(count.front().kind, ::lsp::SymbolKind::Number);
  EXPECT_EQ(count.front().detail, "Int32");
  ASSERT_TRUE(count.front().tags);
  ASSERT_EQ(count.front().tags->size(), 1U);
  EXPECT_EQ(count.front().tags->front(), ::lsp::SymbolTag::Deprecated);
  EXPECT_TRUE(hasItem(
      ws.language("xsmpcat").typeHierarchyProvider->supertypes(count.front()),
      "Int32"));
  const auto ratio = prepare("Ratio");
  ASSERT_EQ(ratio.size(), 1U);
  EXPECT_EQ(ratio.front().kind, ::lsp::SymbolKind::Number);
  EXPECT_EQ(ratio.front().detail, "Float64");
  EXPECT_FALSE(ratio.front().tags);
  EXPECT_TRUE(hasItem(
      ws.language("xsmpcat").typeHierarchyProvider->supertypes(ratio.front()),
      "Float64"));
  const auto primitive = prepare("Int32");
  ASSERT_EQ(primitive.size(), 1U);
  const auto children =
      ws.language("xsmpcat").typeHierarchyProvider->subtypes(primitive.front());
  ASSERT_EQ(children.size(), 1U);
  EXPECT_EQ(children.front().name, "Count");
  EXPECT_EQ(children.front().kind, ::lsp::SymbolKind::Number);
  EXPECT_EQ(children.front().detail, "Int32");
  ASSERT_TRUE(children.front().tags);
  ASSERT_EQ(children.front().tags->size(), 1U);
  EXPECT_EQ(children.front().tags->front(), ::lsp::SymbolTag::Deprecated);
}

TEST(XsmpcatTypeHierarchyProvider, SubtypesIncludeArrayAndValueReferenceUses) {
  auto ws = xsmp::xsmpcatWorkspace();
  const std::string text =
      "catalogue c\nnamespace n { primitive Int32 "
      "array Values = Int32[2] using IntPointer = Int32* }\n";
  const auto document = xsmp::parse(ws, "xsmpcat", text);
  ASSERT_TRUE(document->parseSucceeded());
  const auto item =
      ws.language("xsmpcat").typeHierarchyProvider->prepareTypeHierarchy(
          *document, cursiv::to_lsp(document->textDocument().positionAt(
                         static_cast<cursiv::TextOffset>(text.find("Int32")))));
  ASSERT_EQ(item.size(), 1U);
  const auto children =
      ws.language("xsmpcat").typeHierarchyProvider->subtypes(item.front());
  ASSERT_EQ(children.size(), 2U);
  EXPECT_EQ(children[0].name, "Values");
  EXPECT_EQ(children[0].kind, ::lsp::SymbolKind::Array);
  EXPECT_EQ(children[0].detail, "Int32[2]");
  EXPECT_EQ(children[1].name, "IntPointer");
  EXPECT_EQ(children[1].detail, "Int32*");
}

TEST(XsmpcatTypeHierarchyProvider, ArrayAndValueReferenceGoUpToWhatTheyUse) {
  auto ws = xsmp::xsmpcatWorkspace();
  const std::string text =
      "catalogue c\nnamespace n { primitive Item array Ar = Item[2] using Rf = "
      "Item* }\n";
  const auto document = xsmp::parse(ws, "xsmpcat", text);
  const auto& provider = *ws.language("xsmpcat").typeHierarchyProvider;
  for (const std::string_view name : {"Ar", "Rf"}) {
    const auto items = provider.prepareTypeHierarchy(
        *document, cursiv::to_lsp(document->textDocument().positionAt(
                       static_cast<cursiv::TextOffset>(text.find(name)))));
    ASSERT_EQ(items.size(), 1u);
    const auto supers = provider.supertypes(items.front());
    ASSERT_EQ(supers.size(), 1u) << name;
    EXPECT_EQ(supers.front().name, "Item");
  }
}
