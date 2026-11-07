#include <gtest/gtest.h>

#include <cursiv/parser/Parser.hpp>
#include <cursiv/workspace/Document.hpp>
#include <cursiv/workspace/DocumentBuilder.hpp>
#include <cursiv/workspace/Documents.hpp>
#include <span>
#include <string>
#include <string_view>
#include <xsmp/CoreModule.hpp>
#include <xsmp/SharedCoreServices.hpp>
#include <xsmp/ast.hpp>

#include "support/LspTestSupport.hpp"

namespace {

// The catalogue these tests read.
constexpr std::string_view kSample = R"(catalogue demo

namespace demo
{
    /** @uuid 61e305de-369b-4cc6-8455-f5a16922c1e9 */
    public struct Vector3
    {
        constant Smp.Bool prec = 1 || 2 && 3 == 4 << 5 + 6 * 7
        constant Smp.String8 label = "a\tb\x21"
        field Smp.Float64 x = 0.0
        field Smp.Float64 y = 0.0
        field Smp.Float64 z = 0.0
    }
}

namespace nested
{
}
)";

// Builds the sample document through the core pipeline. Validation is
// disabled: these tests inspect only the parse result.
std::shared_ptr<cursiv::Document> parseSample(
    xsmp::XsmpSharedCoreServices& shared) {
  shared.serviceRegistry->registerServices(xsmp::createXsmpcatServices(shared));
  auto document = shared.documents->createDocument("file:///sample.xsmpcat",
                                                   std::string(kSample));
  auto* raw = document.get();
  shared.documentBuilder->build(std::span(&raw, 1), {.validation = false});
  return document;
}

}  // namespace

// The sample catalogue parses with no syntax errors and yields the expected
// top-level AST.
TEST(XsmpcatGrammar, SampleParsesWithoutSyntaxErrors) {
  auto shared = xsmp::createXsmpSharedServices();
  auto document = parseSample(*shared);

  ASSERT_TRUE(document->parseSucceeded());

  auto* catalogue =
      cursiv::ast_cast<xsmp::Catalogue>(document->parseResult.value);
  ASSERT_NE(catalogue, nullptr);
  EXPECT_EQ(catalogue->name, "demo");
  EXPECT_EQ(catalogue->elements.size(), 2u);
}

// Operator precedence: `1 || 2 && 3 == 4 << 5 + 6 * 7` must associate as
// `1 || (2 && (3 == (4 << (5 + (6 * 7)))))`. The guards were removed, so this
// also proves cursiv disambiguates `||`/`&&`/`<<`/`==` from their single-char
// prefixes.
TEST(XsmpcatGrammar, ExpressionPrecedence) {
  using namespace xsmp;
  auto shared = xsmp::createXsmpSharedServices();
  auto document = parseSample(*shared);
  ASSERT_TRUE(document->parseSucceeded());

  auto* catalogue = cursiv::ast_cast<Catalogue>(document->parseResult.value);
  ASSERT_NE(catalogue, nullptr);

  const auto* demo = catalogue->elements[0];
  ASSERT_NE(demo, nullptr);
  const auto* vector3 = xsmp::findNamed<Structure>(demo->elements, "Vector3");
  ASSERT_NE(vector3, nullptr);
  const auto* prec = xsmp::findNamed<Constant>(vector3->elements, "prec");
  ASSERT_NE(prec, nullptr);
  ASSERT_NE(prec->value, nullptr);

  // top: `||`
  const auto* root = cursiv::ast_cast<const BinaryOperation>(prec->value);
  ASSERT_NE(root, nullptr);
  EXPECT_EQ(root->feature, "||");
  // left of `||` is the literal `1`
  const auto* one = cursiv::ast_cast<const IntegerLiteral>(root->leftOperand);
  ASSERT_NE(one, nullptr);
  EXPECT_EQ(one->text, "1");
  // right of `||` is `&&`
  const auto* andOp =
      cursiv::ast_cast<const BinaryOperation>(root->rightOperand);
  ASSERT_NE(andOp, nullptr);
  EXPECT_EQ(andOp->feature, "&&");
  // right of `&&` is `==`
  const auto* eqOp =
      cursiv::ast_cast<const BinaryOperation>(andOp->rightOperand);
  ASSERT_NE(eqOp, nullptr);
  EXPECT_EQ(eqOp->feature, "==");
  // right of `==` is `<<`
  const auto* shiftOp =
      cursiv::ast_cast<const BinaryOperation>(eqOp->rightOperand);
  ASSERT_NE(shiftOp, nullptr);
  EXPECT_EQ(shiftOp->feature, "<<");
}

// The STRING value converter is wired into the parser: `"a\tb\x21"` is stored
// decoded (tab + '!') in the AST, not as the raw escaped text.
TEST(XsmpcatGrammar, StringLiteralConverterWired) {
  using namespace xsmp;
  auto shared = xsmp::createXsmpSharedServices();
  auto document = parseSample(*shared);
  ASSERT_TRUE(document->parseSucceeded());

  auto* catalogue = cursiv::ast_cast<Catalogue>(document->parseResult.value);
  ASSERT_NE(catalogue, nullptr);
  const auto* demo = catalogue->elements[0];
  ASSERT_NE(demo, nullptr);
  const auto* vector3 = xsmp::findNamed<Structure>(demo->elements, "Vector3");
  ASSERT_NE(vector3, nullptr);
  const auto* label = xsmp::findNamed<Constant>(vector3->elements, "label");
  ASSERT_NE(label, nullptr);

  const auto* str = cursiv::ast_cast<const StringLiteral>(label->value);
  ASSERT_NE(str, nullptr);
  ASSERT_EQ(str->value.size(), 1u);
  EXPECT_EQ(str->value[0], "a\tb!");
}

// The dots of a range never end the number before them.
TEST(XsmpcatGrammar, RangesFollowANumberWithNoBlank) {
  auto ws = xsmp::xsmpcatWorkspace();
  const auto& parser = *ws.language("xsmpcat").parser;
  for (const auto* text : {
           "catalogue C namespace n { model M { container M[1...5] c } }",
           "catalogue C namespace n { integer I in 1...5 }",
           "catalogue C namespace n { float F in 1.0...2.5 }",
           "catalogue C namespace n { float F in 0..<1 }",
           "catalogue C namespace n { float F in 0.5<..1. }",
           "catalogue C namespace n { float F in 0.<.<1.e1 }",
       }) {
    SCOPED_TRACE(text);
    EXPECT_TRUE(parser.parse(text).diagnostics.empty());
  }
  const auto result = parser.parse(
      "catalogue C namespace n { struct S { constant Smp.Float64 c = 1. } }");
  EXPECT_TRUE(result.diagnostics.empty());
  auto found = false;
  for (const auto* node : result.nodes())
    if (const auto* number = cursiv::ast_cast<xsmp::FloatingLiteral>(node))
      found = number->text == "1.";
  EXPECT_TRUE(found);
}
