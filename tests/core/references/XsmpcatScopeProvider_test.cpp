#include <gtest/gtest.h>

#include <cursiv/workspace/DocumentBuilder.hpp>
#include <map>
#include <span>
#include <string>
#include <string_view>
#include <vector>
#include <xsmp/Builtins.hpp>
#include <xsmp/SharedLspServices.hpp>
#include <xsmp/ast.hpp>
#include <xsmp/utils/XsmpUtils.hpp>

#include "support/LspTestSupport.hpp"

namespace {

// Every `DesignatedInitializer` element of `expr` (a `CollectionLiteral`), in
// order. Returns an empty vector if `expr` isn't a `CollectionLiteral`.
std::vector<const xsmp::DesignatedInitializer*> designatedInitializers(
    const xsmp::Expression* expr) {
  using namespace xsmp;
  std::vector<const DesignatedInitializer*> result;
  const auto* collection = cursiv::ast_cast<const CollectionLiteral>(expr);
  if (collection == nullptr) {
    return result;
  }
  for (const auto* element : collection->elements) {
    if (const auto* designated =
            cursiv::ast_cast<const DesignatedInitializer>(element)) {
      result.push_back(designated);
    }
  }
  return result;
}

}  // namespace

TEST(XsmpcatScopeProvider, ExpectedTypeOfPropertyChildrenIsThePropertyType) {
  auto ws = xsmp::xsmpcatWorkspace();
  xsmp::loadXsmpcatBuiltins(ws.shared());
  auto document =
      xsmp::parse(ws, "xsmpcat", R"(catalogue PropertyType namespace n {
    model M { @Attributes.Static public property Smp.Int32 value }
  })");
  ASSERT_TRUE(document->parseSucceeded());
  const auto& language =
      dynamic_cast<const xsmp::XsmpcatCoreServices&>(ws.language("xsmpcat"));
  const xsmp::Property* property = nullptr;
  for (const auto* node : xsmp::descendantsOf(*document->parseResult.value))
    if (const auto* candidate = cursiv::ast_cast<xsmp::Property>(node))
      property = candidate;
  ASSERT_NE(property, nullptr);
  ASSERT_NE(property->type.get(), nullptr);
  ASSERT_EQ(property->attributes.size(), 1u);
  EXPECT_EQ(language.typeProvider->getType(*property->attributes[0]),
            property->type.get());
}

TEST(XsmpcatScopeProvider,
     NumericBooleanAttributesControlStaticFieldTypeInference) {
  auto ws = xsmp::xsmpcatWorkspace();
  xsmp::loadXsmpcatBuiltins(ws.shared());
  auto document =
      xsmp::parse(ws, "xsmpcat", R"(catalogue NumericAttributes namespace n {
    enum A { ValueA = 0 } enum B { ValueB = 0 }
    struct S { @Attributes.Static(1) field A ignored field B selected }
    model M { field S value = { .selected = B.ValueB } }
  })");
  ASSERT_TRUE(document->parseSucceeded());
  const auto& language =
      dynamic_cast<const xsmp::XsmpcatCoreServices&>(ws.language("xsmpcat"));
  const xsmp::DesignatedInitializer* initializer = nullptr;
  const xsmp::Field* ignored = nullptr;
  for (const auto* node : xsmp::descendantsOf(*document->parseResult.value)) {
    if (const auto* candidate =
            cursiv::ast_cast<xsmp::DesignatedInitializer>(node))
      initializer = candidate;
    if (const auto* candidate = cursiv::ast_cast<xsmp::Field>(node);
        candidate && candidate->name == "ignored")
      ignored = candidate;
  }
  ASSERT_NE(initializer, nullptr);
  ASSERT_NE(initializer->expr, nullptr);
  ASSERT_NE(ignored, nullptr);
  EXPECT_TRUE(xsmp::isStaticAttribute(*ignored));
  const auto* type = language.typeProvider->getType(*initializer->expr);
  ASSERT_NE(type, nullptr);
  EXPECT_EQ(type->name, "B");
}

TEST(XsmpcatScopeProvider, ResolvesTypeReferences) {
  using namespace xsmp;
  auto ws = xsmp::xsmpcatWorkspace();
  auto document = xsmp::parse(ws, "xsmpcat",
                              "catalogue test\n"
                              "namespace demo\n"
                              "{\n"
                              "  primitive P\n"
                              "  struct S { field P x }\n"
                              "  namespace sub { primitive Q }\n"
                              "  struct T { field demo.sub.Q y }\n"
                              "}\n");
  ASSERT_TRUE(document->parseSucceeded());
  auto* catalogue = cursiv::ast_cast<Catalogue>(document->parseResult.value);
  ASSERT_NE(catalogue, nullptr);
  ASSERT_FALSE(catalogue->elements.empty());

  const auto* demo = catalogue->elements[0];
  ASSERT_NE(demo, nullptr);

  // simple-name reference within the same namespace
  const auto* structS = xsmp::findNamed<Structure>(demo->elements, "S");
  ASSERT_NE(structS, nullptr);
  const auto* fieldX = xsmp::findNamed<Field>(structS->elements, "x");
  ASSERT_NE(fieldX, nullptr);
  ASSERT_TRUE(fieldX->type) << "field S.x : P should resolve";
  EXPECT_EQ(fieldX->type.get()->name, "P");

  // fully-qualified reference into a nested namespace
  const auto* structT = xsmp::findNamed<Structure>(demo->elements, "T");
  ASSERT_NE(structT, nullptr);
  const auto* fieldY = xsmp::findNamed<Field>(structT->elements, "y");
  ASSERT_NE(fieldY, nullptr);
  ASSERT_TRUE(fieldY->type) << "field T.y : demo.sub.Q should resolve";
  EXPECT_EQ(fieldY->type.get()->name, "Q");
}

TEST(XsmpcatScopeProvider, ReportsUnresolvedTypeReference) {
  using namespace xsmp;
  auto ws = xsmp::xsmpcatWorkspace();
  auto document = xsmp::parse(ws, "xsmpcat",
                              "catalogue test\n"
                              "namespace demo\n"
                              "{\n"
                              "  struct S { field Missing x }\n"
                              "}\n");
  ASSERT_TRUE(document->parseSucceeded());
  auto* catalogue = cursiv::ast_cast<Catalogue>(document->parseResult.value);
  ASSERT_NE(catalogue, nullptr);
  const auto* demo = catalogue->elements[0];
  const auto* structS = xsmp::findNamed<Structure>(demo->elements, "S");
  ASSERT_NE(structS, nullptr);
  const auto* fieldX = xsmp::findNamed<Field>(structS->elements, "x");
  ASSERT_NE(fieldX, nullptr);
  EXPECT_FALSE(fieldX->type) << "field S.x : Missing must stay unresolved";
}

TEST(XsmpcatScopeProvider, ResolvesMemberReferences) {
  using namespace xsmp;
  auto ws = xsmp::xsmpcatWorkspace();
  auto document = xsmp::parse(
      ws, "xsmpcat",
      "catalogue test\n"
      "namespace demo\n"
      "{\n"
      "  primitive Bool\n"
      "  enum Mode { Standby = 0, Nominal = 1 }\n"
      "  struct S\n"
      "  {\n"
      "    field Mode m = Mode.Nominal\n"  // qualified enum literal
      "    field Mode n = Nominal\n"  // bare enum literal, NOT in scope here
      "  }\n"
      "  class C\n"
      "  {\n"
      "    field Bool f\n"
      "    public property Bool p -> f\n"  // attached field, simple name in
                                           // type
      "  }\n"
      "}\n");
  ASSERT_TRUE(document->parseSucceeded());
  auto* catalogue = cursiv::ast_cast<Catalogue>(document->parseResult.value);
  ASSERT_NE(catalogue, nullptr);
  const auto* demo = catalogue->elements[0];

  const auto* structS = xsmp::findNamed<Structure>(demo->elements, "S");
  ASSERT_NE(structS, nullptr);

  const auto* fieldM = xsmp::findNamed<Field>(structS->elements, "m");
  ASSERT_NE(fieldM, nullptr);
  const auto* refM =
      cursiv::ast_cast<const NamedElementReference>(fieldM->default_);
  ASSERT_NE(refM, nullptr);
  ASSERT_TRUE(refM->value) << "Mode.Nominal should resolve";
  EXPECT_EQ(refM->value.get<EnumerationLiteral>()->name, "Nominal");

  const auto* fieldN = xsmp::findNamed<Field>(structS->elements, "n");
  ASSERT_NE(fieldN, nullptr);
  const auto* refN =
      cursiv::ast_cast<const NamedElementReference>(fieldN->default_);
  ASSERT_NE(refN, nullptr);
  EXPECT_FALSE(refN->value)
      << "bare enum literal must stay unresolved outside the Mode enum's own "
         "body — only Mode.Nominal is visible from the surrounding namespace";

  const auto* classC = xsmp::findNamed<Class>(demo->elements, "C");
  ASSERT_NE(classC, nullptr);
  const auto* propP = xsmp::findNamed<Property>(classC->elements, "p");
  ASSERT_NE(propP, nullptr);
  ASSERT_TRUE(propP->attachedField.has_value());
  ASSERT_TRUE(*propP->attachedField) << "attached field -> f should resolve";
  EXPECT_EQ(propP->attachedField->get()->name, "f");
}

// A bare enum literal IS visible from inside the SAME enum's own body (e.g. a
// later literal's value expression referencing an earlier sibling) — the
// bare-name scope stays owned by the Enumeration node itself, never promoted
// to the enclosing namespace/catalogue.
TEST(XsmpcatScopeProvider, BareEnumLiteralResolvesWithinItsOwnEnumBody) {
  using namespace xsmp;
  auto ws = xsmp::xsmpcatWorkspace();
  auto document = xsmp::parse(
      ws, "xsmpcat",
      "catalogue test\n"
      "namespace demo\n"
      "{\n"
      "  integer Int32\n"
      "  enum Mode { A = 0, B = A + 1 }\n"  // B's value references bare A
      "}\n");
  ASSERT_TRUE(document->parseSucceeded());
  auto* catalogue = cursiv::ast_cast<Catalogue>(document->parseResult.value);
  ASSERT_NE(catalogue, nullptr);
  const auto* demo = catalogue->elements[0];

  const auto* mode = xsmp::findNamed<Enumeration>(demo->elements, "Mode");
  ASSERT_NE(mode, nullptr);
  const EnumerationLiteral* literalB = nullptr;
  for (const auto* literal : mode->literal)
    if (literal->name == "B") {
      literalB = literal;
      break;
    }
  ASSERT_NE(literalB, nullptr);
  const auto* binary = cursiv::ast_cast<const BinaryOperation>(literalB->value);
  ASSERT_NE(binary, nullptr);
  const auto* refA =
      cursiv::ast_cast<const NamedElementReference>(binary->leftOperand);
  ASSERT_NE(refA, nullptr);
  ASSERT_TRUE(refA->value) << "bare A should resolve within Mode's own body";
  EXPECT_EQ(refA->value.get<EnumerationLiteral>()->name, "A");
}

// `NamedElementReference::value` is a `reference<NamedElement>`: a same-named
// Type must never satisfy it, only an actual Constant or EnumerationLiteral.
TEST(XsmpcatScopeProvider, ConstantExpressionDoesNotResolveToSameNamedType) {
  using namespace xsmp;
  auto ws = xsmp::xsmpcatWorkspace();
  auto document = xsmp::parse(
      ws, "xsmpcat",
      "catalogue test\n"
      "namespace demo\n"
      "{\n"
      "  integer Int32\n"
      "  struct Same {}\n"
      "  struct S { constant Int32 Same = 1 constant Int32 c = Same }\n"
      "}\n");
  ASSERT_TRUE(document->parseSucceeded());
  auto* catalogue = cursiv::ast_cast<Catalogue>(document->parseResult.value);
  ASSERT_NE(catalogue, nullptr);
  const auto* demo = catalogue->elements[0];

  const auto* structS = xsmp::findNamed<Structure>(demo->elements, "S");
  ASSERT_NE(structS, nullptr);
  const auto* constantC = xsmp::findNamed<Constant>(structS->elements, "c");
  ASSERT_NE(constantC, nullptr);
  const auto* refSame =
      cursiv::ast_cast<const NamedElementReference>(constantC->value);
  ASSERT_NE(refSame, nullptr);
  ASSERT_TRUE(refSame->value)
      << "bare Same should resolve to S's own constant, not the struct";
  EXPECT_EQ(refSame->value.get<Constant>()->name, "Same");
  EXPECT_NE(refSame->value.get<Constant>(), nullptr)
      << "must resolve to the Constant, never the same-named Structure";
}

TEST(XsmpcatScopeProvider, ResolvesInheritedMembers) {
  using namespace xsmp;
  auto ws = xsmp::xsmpcatWorkspace();
  auto document = xsmp::parse(
      ws, "xsmpcat",
      "catalogue test\n"
      "namespace demo\n"
      "{\n"
      "  primitive Bool\n"
      "  class Base { field Bool inherited }\n"
      "  class Derived extends Base\n"
      "  {\n"
      "    public property Bool p -> inherited\n"  // field from the base class
      "  }\n"
      "}\n");
  ASSERT_TRUE(document->parseSucceeded());
  auto* catalogue = cursiv::ast_cast<Catalogue>(document->parseResult.value);
  ASSERT_NE(catalogue, nullptr);
  const auto* demo = catalogue->elements[0];

  const auto* derived = xsmp::findNamed<Class>(demo->elements, "Derived");
  ASSERT_NE(derived, nullptr);
  ASSERT_TRUE(derived->base.has_value());
  ASSERT_TRUE(*derived->base) << "extends Base should resolve";

  const auto* propP = xsmp::findNamed<Property>(derived->elements, "p");
  ASSERT_NE(propP, nullptr);
  ASSERT_TRUE(propP->attachedField.has_value());
  ASSERT_TRUE(*propP->attachedField)
      << "attached field -> inherited (from Base) should resolve";
  EXPECT_EQ(propP->attachedField->get()->name, "inherited");
}

// `.field = value` resolves against the fields of the literal's own declared
// type, even when that type is not a lexical ancestor of the literal (Vec is a
// sibling of Holder, not an enclosing declaration).
TEST(XsmpcatScopeProvider,
     DesignatedInitializerFieldResolvesAgainstFieldsOwnDeclaredType) {
  using namespace xsmp;
  auto ws = xsmp::xsmpcatWorkspace();
  auto document =
      xsmp::parse(ws, "xsmpcat",
                  "catalogue test\n"
                  "namespace demo\n"
                  "{\n"
                  "  primitive Float64\n"
                  "  struct Vec { field Float64 x field Float64 y }\n"
                  "  struct Holder { field Vec v = { .x = 1.0, .y = 2.0 } }\n"
                  "}\n");
  ASSERT_TRUE(document->parseSucceeded());
  auto* catalogue = cursiv::ast_cast<Catalogue>(document->parseResult.value);
  ASSERT_NE(catalogue, nullptr);
  const auto* demo = catalogue->elements[0];

  const auto* holder = xsmp::findNamed<Structure>(demo->elements, "Holder");
  ASSERT_NE(holder, nullptr);
  const auto* fieldV = xsmp::findNamed<Field>(holder->elements, "v");
  ASSERT_NE(fieldV, nullptr);
  const auto initializers = designatedInitializers(fieldV->default_);
  ASSERT_EQ(initializers.size(), 2u);
  ASSERT_TRUE(initializers[0]->field) << ".x should resolve against Vec.x";
  EXPECT_EQ(initializers[0]->field.get()->name, "x");
  ASSERT_TRUE(initializers[1]->field) << ".y should resolve against Vec.y";
  EXPECT_EQ(initializers[1]->field.get()->name, "y");
}

// `.field` resolves only against the literal's expected type, never against an
// enclosing scope, even when that scope has a same-named symbol. Holder has its
// own field named "x" (a decoy) and a field of type Vec initialized via
// `{.x=1.0}`; the reference must resolve to Vec.x.
TEST(XsmpcatScopeProvider,
     DesignatedInitializerDoesNotFallBackToEnclosingScopeOnNameCollision) {
  using namespace xsmp;
  auto ws = xsmp::xsmpcatWorkspace();
  auto document = xsmp::parse(
      ws, "xsmpcat",
      "catalogue test\n"
      "namespace demo\n"
      "{\n"
      "  primitive Float64\n"
      "  struct Vec { field Float64 x }\n"
      "  struct Holder { field Float64 x field Vec v = { .x = 1.0 } }\n"
      "}\n");
  ASSERT_TRUE(document->parseSucceeded());
  auto* catalogue = cursiv::ast_cast<Catalogue>(document->parseResult.value);
  ASSERT_NE(catalogue, nullptr);
  const auto* demo = catalogue->elements[0];

  const auto* vec = xsmp::findNamed<Structure>(demo->elements, "Vec");
  ASSERT_NE(vec, nullptr);
  const auto* vecX = xsmp::findNamed<Field>(vec->elements, "x");
  ASSERT_NE(vecX, nullptr);

  const auto* holder = xsmp::findNamed<Structure>(demo->elements, "Holder");
  ASSERT_NE(holder, nullptr);
  const auto* fieldV = xsmp::findNamed<Field>(holder->elements, "v");
  ASSERT_NE(fieldV, nullptr);
  const auto initializers = designatedInitializers(fieldV->default_);
  ASSERT_EQ(initializers.size(), 1u);
  ASSERT_TRUE(initializers[0]->field);
  EXPECT_EQ(initializers[0]->field.get(), vecX)
      << ".x must resolve to Vec's own field, not Holder's decoy field of "
         "the same simple name";
}

// An unknown field name stays unresolved (not silently matched against an
// unrelated same-named global/lexical symbol).
TEST(XsmpcatScopeProvider, DesignatedInitializerUnknownFieldNameIsUnresolved) {
  using namespace xsmp;
  auto ws = xsmp::xsmpcatWorkspace();
  auto document =
      xsmp::parse(ws, "xsmpcat",
                  "catalogue test\n"
                  "namespace demo\n"
                  "{\n"
                  "  primitive Float64\n"
                  "  struct Vec { field Float64 x }\n"
                  "  struct Holder { field Vec v = { .bogus = 1.0 } }\n"
                  "}\n");
  ASSERT_TRUE(document->parseSucceeded());
  auto* catalogue = cursiv::ast_cast<Catalogue>(document->parseResult.value);
  ASSERT_NE(catalogue, nullptr);
  const auto* demo = catalogue->elements[0];

  const auto* holder = xsmp::findNamed<Structure>(demo->elements, "Holder");
  ASSERT_NE(holder, nullptr);
  const auto* fieldV = xsmp::findNamed<Field>(holder->elements, "v");
  ASSERT_NE(fieldV, nullptr);
  const auto initializers = designatedInitializers(fieldV->default_);
  ASSERT_EQ(initializers.size(), 1u);
  EXPECT_FALSE(initializers[0]->field) << ".bogus must stay unresolved";
}

// A designated initializer on a field whose type is not a Structure never
// resolves.
TEST(XsmpcatScopeProvider,
     DesignatedInitializerOnNonStructureTypeIsUnresolved) {
  using namespace xsmp;
  auto ws = xsmp::xsmpcatWorkspace();
  auto document = xsmp::parse(ws, "xsmpcat",
                              "catalogue test\n"
                              "namespace demo\n"
                              "{\n"
                              "  integer Int32\n"
                              "  struct Holder { field Int32 x = { .y = 1 } }\n"
                              "}\n");
  ASSERT_TRUE(document->parseSucceeded());
  auto* catalogue = cursiv::ast_cast<Catalogue>(document->parseResult.value);
  ASSERT_NE(catalogue, nullptr);
  const auto* demo = catalogue->elements[0];

  const auto* holder = xsmp::findNamed<Structure>(demo->elements, "Holder");
  ASSERT_NE(holder, nullptr);
  const auto* fieldX = xsmp::findNamed<Field>(holder->elements, "x");
  ASSERT_NE(fieldX, nullptr);
  const auto initializers = designatedInitializers(fieldX->default_);
  ASSERT_EQ(initializers.size(), 1u);
  EXPECT_FALSE(initializers[0]->field)
      << ".y on an Int32-typed field must stay unresolved";
}

// `.field` resolves through the base chain of a Class too.
TEST(XsmpcatScopeProvider,
     DesignatedInitializerResolvesInheritedFieldThroughClassBaseChain) {
  using namespace xsmp;
  auto ws = xsmp::xsmpcatWorkspace();
  auto document =
      xsmp::parse(ws, "xsmpcat",
                  "catalogue test\n"
                  "namespace demo\n"
                  "{\n"
                  "  primitive Int32\n"
                  "  class Base { field Int32 a }\n"
                  "  class Derived extends Base { field Int32 b }\n"
                  "  struct Holder { field Derived d = { .a = 1, .b = 2 } }\n"
                  "}\n");
  ASSERT_TRUE(document->parseSucceeded());
  auto* catalogue = cursiv::ast_cast<Catalogue>(document->parseResult.value);
  ASSERT_NE(catalogue, nullptr);
  const auto* demo = catalogue->elements[0];

  const auto* holder = xsmp::findNamed<Structure>(demo->elements, "Holder");
  ASSERT_NE(holder, nullptr);
  const auto* fieldD = xsmp::findNamed<Field>(holder->elements, "d");
  ASSERT_NE(fieldD, nullptr);
  const auto initializers = designatedInitializers(fieldD->default_);
  ASSERT_EQ(initializers.size(), 2u);
  ASSERT_TRUE(initializers[0]->field) << ".a should resolve via Base";
  EXPECT_EQ(initializers[0]->field.get()->name, "a");
  ASSERT_TRUE(initializers[1]->field) << ".b should resolve on Derived itself";
  EXPECT_EQ(initializers[1]->field.get()->name, "b");
}

// `xsmp:/`-scheme documents (the real embedding scheme `Builtins.cpp`
// uses) are visible from a standalone (project-less) document via
// `ProjectManager`'s standalone-visibility fallback.
TEST(XsmpcatScopeProvider, ResolvesAgainstBuiltins) {
  using namespace xsmp;
  auto ws = xsmp::xsmpcatWorkspace();
  xsmp::loadXsmpcatBuiltins(ws.shared<xsmp::XsmpSharedServices>());

  auto document = xsmp::parse(ws, "xsmpcat",
                              "catalogue test\n"
                              "namespace demo\n"
                              "{\n"
                              "  struct S\n"
                              "  {\n"
                              "    field Smp.Bool b\n"  // qualified
                              "    field Bool c\n"      // Smp.* shortcut
                              "  }\n"
                              "}\n");
  ASSERT_TRUE(document->parseSucceeded());
  auto* catalogue = cursiv::ast_cast<Catalogue>(document->parseResult.value);
  ASSERT_NE(catalogue, nullptr);
  const auto* demo = catalogue->elements[0];
  const auto* structS = xsmp::findNamed<Structure>(demo->elements, "S");
  ASSERT_NE(structS, nullptr);

  const auto* fieldB = xsmp::findNamed<Field>(structS->elements, "b");
  ASSERT_NE(fieldB, nullptr);
  ASSERT_TRUE(fieldB->type) << "Smp.Bool should resolve against the builtin";
  EXPECT_EQ(fieldB->type.get()->name, "Bool");

  const auto* fieldC = xsmp::findNamed<Field>(structS->elements, "c");
  ASSERT_NE(fieldC, nullptr);
  ASSERT_TRUE(fieldC->type) << "Bool (Smp.* shortcut) should resolve";
  EXPECT_EQ(fieldC->type.get()->name, "Bool");
}

TEST(XsmpcatScopeProvider, QualifiedBuiltinAliasesRetainAllSuffixSegments) {
  auto ws = xsmp::xsmpcatWorkspace();
  xsmp::loadXsmpcatBuiltins(ws.shared<xsmp::XsmpSharedServices>());
  auto result = cursiv::test::validationHelper(ws.language("xsmpcat"),
                                               R"(catalogue BuiltinAliases
namespace demo {
 /** @uuid f397f918-6730-4855-ae44-383b9bfc9a88 */
 public model Root { @View(ViewKind.VK_All) field Int32 value }
})");
  for (const auto& diagnostic : result.document->diagnostics)
    EXPECT_EQ(diagnostic.message.find("Could not resolve"), std::string::npos)
        << diagnostic.message;
  const auto* catalogue =
      cursiv::ast_cast<xsmp::Catalogue>(result.document->parseResult.value);
  ASSERT_NE(catalogue, nullptr);
  const auto* model =
      cursiv::ast_cast<xsmp::Model>(catalogue->elements[0]->elements[0]);
  ASSERT_NE(model, nullptr);
  const auto* field = cursiv::ast_cast<xsmp::Field>(model->elements[0]);
  ASSERT_NE(field, nullptr);
  ASSERT_EQ(field->attributes.size(), 1u);
  const auto* literal = cursiv::ast_cast<xsmp::NamedElementReference>(
      field->attributes[0]->value);
  ASSERT_NE(literal, nullptr);
  ASSERT_NE(literal->value.get<xsmp::EnumerationLiteral>(), nullptr);
  EXPECT_EQ(literal->value.get<xsmp::EnumerationLiteral>()->name, "VK_All");
}

TEST(XsmpcatScopeProvider,
     QualifiedAttachedFieldsResolveInOwnerAndInheritedScopes) {
  auto ws = xsmp::xsmpcatWorkspace();
  xsmp::loadXsmpcatBuiltins(ws.shared<xsmp::XsmpSharedServices>());
  auto result = cursiv::test::validationHelper(
      ws.language("xsmpcat"), R"(catalogue LocalFields namespace demo {
 public class Base { public field Smp.Int32 value property Smp.Int32 direct -> Base.value }
 public class Derived extends Base { property Smp.Int32 inherited -> demo.Base.value }
})");
  for (const auto& diagnostic : result.document->diagnostics)
    EXPECT_EQ(diagnostic.message.find("Could not resolve"), std::string::npos)
        << diagnostic.message;
  unsigned properties = 0;
  for (const auto* node :
       xsmp::descendantsOf(*result.document->parseResult.value))
    if (const auto* property = cursiv::ast_cast<xsmp::Property>(node)) {
      ASSERT_TRUE(property->attachedField);
      ASSERT_NE(property->attachedField->get(), nullptr);
      EXPECT_EQ(property->attachedField->get()->name, "value");
      ++properties;
    }
  EXPECT_EQ(properties, 2u);
}

// A relink keeps the names of its parse: they live in a pool only a new parse
// frees, so making them again would grow it with every edit elsewhere.
TEST(XsmpcatScopeProvider, RelinkingKeepsTheDottedNamesOfTheParse) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto& shared = ws.shared();
  const auto document = shared.documents->createDocument(
      "file:///dotted/a.xsmpcat",
      "catalogue A namespace outer { namespace inner { enum E { One } } }");
  auto* raw = document.get();
  shared.documentBuilder->build(std::span(&raw, 1));
  ASSERT_NE(cursiv::ast_cast<xsmp::Catalogue>(document->parseResult.value),
            nullptr);
  // A new parse makes a new catalogue, so it is read each time.
  const auto dotted = [&] {
    std::map<std::string, const char*> names;
    for (const auto& entry :
         document->localSymbols.forContainer(document->parseResult.value))
      names.emplace(std::string(entry.name), entry.name.data());
    return names;
  };
  const auto before = dotted();
  EXPECT_TRUE(before.contains("outer.inner.E.One")) << before.size();
  shared.documentBuilder->resetToState(*document,
                                       cursiv::DocumentState::ComputedScopes);
  shared.documentBuilder->update({}, {});
  ASSERT_GE(document->state.load(), cursiv::DocumentState::IndexedReferences);
  EXPECT_EQ(dotted(), before);

  // A new parse makes them again.
  xsmp::parse(
      ws, "xsmpcat",
      "catalogue A namespace outer { namespace inner { enum E { Two } } }",
      document->uri);
  EXPECT_FALSE(dotted().contains("outer.inner.E.One"));
  EXPECT_TRUE(dotted().contains("outer.inner.E.Two"));
}
