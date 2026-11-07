#include <gtest/gtest.h>

#include <cmath>
#include <cstdint>
#include <cursiv/validation/ValidationAcceptor.hpp>
#include <cursiv/workspace/Document.hpp>
#include <limits>
#include <map>
#include <string>
#include <string_view>
#include <vector>
#include <xsmp/Builtins.hpp>
#include <xsmp/CliWorkspace.hpp>
#include <xsmp/LspModule.hpp>
#include <xsmp/ast.hpp>
#include <xsmp/solver/Solver.hpp>
#include <xsmp/utils/XsmpDocumentation.hpp>

#include "support/DiagnosticTestSupport.hpp"

namespace {

const xsmp::Expression* constantValue(const xsmp::Catalogue& catalogue,
                                      std::string_view name) {
  for (const auto* node : xsmp::descendantsOf(catalogue)) {
    if (const auto* constant = cursiv::ast_cast<const xsmp::Constant>(node);
        constant != nullptr && constant->name == name) {
      return constant->value;
    }
  }
  return nullptr;
}

std::int64_t asInt(const xsmp::SolverValue& value) {
  return static_cast<std::int64_t>(value.integral);
}

}  // namespace

class SolverTest : public ::testing::Test {
 protected:
  void SetUp() override {
    _ws = xsmp::TestWorkspace::withShared<xsmp::XsmpSharedServices>();
    auto& shared = _ws.shared<xsmp::XsmpSharedServices>();
    shared.projectManager = std::make_unique<xsmp::ProjectManager>(shared);
    _ws.registerLanguage(xsmp::createXsmpcatServices(shared));
    _document = xsmp::parse(_ws, "xsmpcat",
                            "catalogue test\n"
                            "namespace demo\n"
                            "{\n"
                            "  primitive I32\n"
                            "  enum Color { Red = 0, Green = 1, Blue = 2 }\n"
                            "  struct K\n"
                            "  {\n"
                            "    constant I32 hex = 0xFF\n"
                            "    constant I32 micros = 42us\n"
                            "    constant I32 oct = 017\n"
                            "    constant I32 grouped = 1'000\n"
                            "    constant I32 arith = 1 + 2 * 3\n"
                            "    constant I32 division = 10 / 3\n"
                            "    constant I32 remainder = 7 % 3\n"
                            "    constant I32 shift = 1 << 4\n"
                            "    constant I32 bitor = 240 | 15\n"
                            "    constant I32 negative = 0 - 5\n"
                            "    constant I32 complement = ~ 0\n"
                            "    constant I32 compare = 5 > 3\n"
                            "    constant I32 logical = true && false\n"
                            "    constant I32 ratio = 3.0 / 2.0\n"
                            "    constant I32 single = 2.5f\n"
                            "    constant I32 piConst = $PI\n"
                            "    constant I32 root = $sqrt(16.0)\n"
                            "    constant I32 colorRef = Color.Green\n"
                            "  }\n"
                            "}\n");
    _catalogue =
        cursiv::ast_cast<xsmp::Catalogue>(_document->parseResult.value);
  }

  std::optional<xsmp::SolverValue> eval(std::string_view constant) {
    return xsmp::getValue(constantValue(*_catalogue, constant));
  }

  xsmp::TestWorkspace _ws;
  std::shared_ptr<cursiv::Document> _document;
  const xsmp::Catalogue* _catalogue = nullptr;
};

TEST_F(SolverTest, IntegerLiterals) {
  ASSERT_TRUE(_catalogue);
  EXPECT_EQ(asInt(*eval("hex")), 255);
  EXPECT_EQ(asInt(*eval("micros")), 42'000);  // 42 * 1000 ns
  EXPECT_EQ(asInt(*eval("oct")), 15);         // 017 octal
  EXPECT_EQ(asInt(*eval("grouped")), 1'000);  // digit separators stripped
  EXPECT_EQ(eval("hex")->type, xsmp::PTK::Int32);
}

TEST_F(SolverTest, Arithmetic) {
  EXPECT_EQ(asInt(*eval("arith")), 7);        // 1 + 2 * 3
  EXPECT_EQ(asInt(*eval("division")), 3);     // 10 / 3
  EXPECT_EQ(asInt(*eval("remainder")), 1);    // 7 % 3
  EXPECT_EQ(asInt(*eval("shift")), 16);       // 1 << 4
  EXPECT_EQ(asInt(*eval("bitor")), 255);      // 240 | 15
  EXPECT_EQ(asInt(*eval("negative")), -5);    // 0 - 5
  EXPECT_EQ(asInt(*eval("complement")), -1);  // ~0
}

TEST_F(SolverTest, BooleanResults) {
  auto compare = eval("compare");
  ASSERT_TRUE(compare);
  EXPECT_EQ(compare->type, xsmp::PTK::Bool);
  EXPECT_TRUE(compare->boolean);  // 5 > 3

  auto logical = eval("logical");
  ASSERT_TRUE(logical);
  EXPECT_EQ(logical->type, xsmp::PTK::Bool);
  EXPECT_FALSE(logical->boolean);  // true && false
}

TEST_F(SolverTest, Floats) {
  auto ratio = eval("ratio");
  ASSERT_TRUE(ratio);
  EXPECT_EQ(ratio->type, xsmp::PTK::Float64);
  EXPECT_DOUBLE_EQ(ratio->floating, 1.5);

  auto single = eval("single");
  ASSERT_TRUE(single);
  EXPECT_EQ(single->type, xsmp::PTK::Float32);
  EXPECT_FLOAT_EQ(static_cast<float>(single->floating), 2.5F);
}

TEST_F(SolverTest, BuiltIns) {
  auto pi = eval("piConst");
  ASSERT_TRUE(pi);
  EXPECT_EQ(pi->type, xsmp::PTK::Float64);
  EXPECT_NEAR(pi->floating, 3.14159265358979, 1e-9);

  auto root = eval("root");
  ASSERT_TRUE(root);
  EXPECT_DOUBLE_EQ(root->floating, 4.0);  // sqrt(16.0)
}

TEST_F(SolverTest, EnumerationReference) {
  auto color = eval("colorRef");
  ASSERT_TRUE(color);
  EXPECT_EQ(color->type, xsmp::PTK::Enum);
  ASSERT_NE(color->literal, nullptr);
  EXPECT_EQ(color->literal->name, "Green");
}

TEST(Solver, IsoDateTimesRetainNanosecondsAndOffsets) {
  auto ws = xsmp::TestWorkspace::withShared<xsmp::XsmpSharedServices>();
  auto& shared = ws.shared<xsmp::XsmpSharedServices>();
  shared.projectManager = std::make_unique<xsmp::ProjectManager>(shared);
  ws.registerLanguage(xsmp::createXsmpcatServices(shared));
  auto document = xsmp::parse(ws, "xsmpcat", R"(catalogue Dates namespace n {
   struct K {
    constant Smp.String8 utc = "2022-10-22T09:07:44.101010101Z"
    constant Smp.String8 offset = "2022-10-22T11:07:44.101010101+02:00"
    constant Smp.String8 before = "1969-12-31T23:59:59.5Z"
    constant Smp.String8 invalid = "2022-10-22T09:07:60Z"
   }
  })");
  const auto* catalogue =
      cursiv::ast_cast<xsmp::Catalogue>(document->parseResult.value);
  ASSERT_NE(catalogue, nullptr);
  for (auto name : {"utc", "offset"}) {
    const auto value =
        xsmp::getValueAs(constantValue(*catalogue, name), xsmp::PTK::DateTime);
    ASSERT_TRUE(value);
    EXPECT_EQ(asInt(*value), 719701664101010101LL);
  }
  const auto before = xsmp::getValueAs(constantValue(*catalogue, "before"),
                                       xsmp::PTK::DateTime);
  ASSERT_TRUE(before);
  EXPECT_EQ(asInt(*before), -946728000500000000LL);
  EXPECT_FALSE(xsmp::getValueAs(constantValue(*catalogue, "invalid"),
                                xsmp::PTK::DateTime));
}

TEST(Solver, WideProductsAndShiftsWrapAndHugeDurationsAreInvalid) {
  auto ws = xsmp::TestWorkspace::withShared<xsmp::XsmpSharedServices>();
  auto& shared = ws.shared<xsmp::XsmpSharedServices>();
  shared.projectManager = std::make_unique<xsmp::ProjectManager>(shared);
  ws.registerLanguage(xsmp::createXsmpcatServices(shared));
  auto document =
      xsmp::parse(ws, "xsmpcat", R"(catalogue Arithmetic namespace n {
   struct K {
    constant Smp.UInt64 product = 0xffffffffffffffffUL * 0xffffffffffffffffUL
    constant Smp.UInt64 wideShift = 1UL << 128
    constant Smp.Int64 signedShift = -1L << 63
    constant Smp.Int64 rightShift = -1L >> 128
    constant Smp.String8 largeDuration = "P999999999999999999999999999999999D"
    constant Smp.String8 outsideInt64 = "P106752D"
    constant Smp.String8 insideInt64 = "P106751D"
   }
  })");
  const auto* catalogue =
      cursiv::ast_cast<xsmp::Catalogue>(document->parseResult.value);
  ASSERT_NE(catalogue, nullptr);
  const auto product = xsmp::getValue(constantValue(*catalogue, "product"));
  const auto wide = xsmp::getValue(constantValue(*catalogue, "wideShift"));
  const auto signedShift =
      xsmp::getValue(constantValue(*catalogue, "signedShift"));
  const auto rightShift =
      xsmp::getValue(constantValue(*catalogue, "rightShift"));
  ASSERT_TRUE(product);
  EXPECT_EQ(product->integral, 1u);
  ASSERT_TRUE(wide);
  EXPECT_EQ(wide->integral, 0u);
  ASSERT_TRUE(signedShift);
  EXPECT_EQ(asInt(*signedShift), std::numeric_limits<std::int64_t>::min());
  ASSERT_TRUE(rightShift);
  EXPECT_EQ(asInt(*rightShift), -1);
  // A Duration of more than 64 bits, or whose total in nanoseconds does not
  // fit an Int64, is invalid.
  EXPECT_FALSE(xsmp::getValueAs(constantValue(*catalogue, "largeDuration"),
                                xsmp::PTK::Duration));
  EXPECT_FALSE(xsmp::getValueAs(constantValue(*catalogue, "outsideInt64"),
                                xsmp::PTK::Duration));
  const auto inside = xsmp::getValueAs(constantValue(*catalogue, "insideInt64"),
                                       xsmp::PTK::Duration);
  ASSERT_TRUE(inside);
  EXPECT_EQ(asInt(*inside), 9223286400000000000LL);
}

TEST(Solver, DurationValidationFollowsTheDayTimeDurationOfTheStandard) {
  for (auto text : {"P0D", "PT0S", "PT0.000000001S", "PT0.1S", "PT1S", "-PT1S",
                    "P2DT1M", "-P2DT1M0.000000001S", "PT1.1234567891S"})
    EXPECT_TRUE(xsmp::isValidDuration(text)) << text;
  for (auto text : {"P2", "PT", "garbagePT1Sgarbage", "PT1H30", "pt1s", "+PT1S",
                    "PT+1S", "PT-1S", "P-2D", "PT1,5S", "PT1.S", "P106752D"})
    EXPECT_FALSE(xsmp::isValidDuration(text)) << text;
  EXPECT_EQ(xsmp::durationNanoseconds("-PT1.5S"), -1'500'000'000);
  EXPECT_FALSE(xsmp::durationNanoseconds("PT"));
}

// A DateTime counts nanoseconds from 2000-01-01T12:00:00Z and holds 64 bits of
// them, roughly from 1710 to 2290.
TEST(Solver, DateTimeCountsFromTheStandardReferenceTime) {
  auto ws = xsmp::TestWorkspace::withShared<xsmp::XsmpSharedServices>();
  auto& shared = ws.shared<xsmp::XsmpSharedServices>();
  shared.projectManager = std::make_unique<xsmp::ProjectManager>(shared);
  ws.registerLanguage(xsmp::createXsmpcatServices(shared));
  auto document =
      xsmp::parse(ws, "xsmpcat", R"(catalogue Reference namespace n {
   struct K {
    constant Smp.String8 origin = "2000-01-01T12:00:00Z"
    constant Smp.String8 before = "2000-01-01T11:59:59Z"
   }
  })");
  const auto* catalogue =
      cursiv::ast_cast<xsmp::Catalogue>(document->parseResult.value);
  ASSERT_NE(catalogue, nullptr);
  const auto origin = xsmp::getValueAs(constantValue(*catalogue, "origin"),
                                       xsmp::PTK::DateTime);
  ASSERT_TRUE(origin);
  EXPECT_EQ(asInt(*origin), 0);
  const auto before = xsmp::getValueAs(constantValue(*catalogue, "before"),
                                       xsmp::PTK::DateTime);
  ASSERT_TRUE(before);
  EXPECT_EQ(asInt(*before), -1'000'000'000);
  EXPECT_TRUE(xsmp::isValidDateTime("2270-01-01T00:00:00Z"));
  EXPECT_FALSE(xsmp::isValidDateTime("1690-01-01T00:00:00Z"));
}

namespace {

class SolverExpressions : public ::testing::Test {
 protected:
  xsmp::TestWorkspace ws =
      xsmp::TestWorkspace::withShared<xsmp::XsmpSharedServices>();

  std::shared_ptr<cursiv::Document> document;

  // With `builtins`, constants have their `Smp` types, which a read of a
  // Constant needs.
  void parse(std::string_view body, bool builtins = false) {
    auto& shared = ws.shared<xsmp::XsmpSharedServices>();
    shared.projectManager = std::make_unique<xsmp::ProjectManager>(shared);
    ws.registerLanguage(xsmp::createXsmpcatServices(shared));
    if (builtins) xsmp::loadXsmpcatBuiltins(shared);
    document =
        xsmp::parse(ws, "xsmpcat",
                    "catalogue test namespace demo { public interface I { "
                        + std::string(body) + " } }");
    ASSERT_TRUE(document->parseSucceeded());
  }

  const xsmp::Expression* expression(std::string_view name) const {
    const auto* catalogue =
        cursiv::ast_cast<xsmp::Catalogue>(document->parseResult.value);
    return catalogue ? constantValue(*catalogue, name) : nullptr;
  }
};

}  // namespace

TEST_F(SolverExpressions, EveryComparisonOperator) {
  parse(R"(
    constant Bool intLess = 1 < 2
    constant Bool intLessFalse = 2 < 1
    constant Bool intGreater = 2 > 1
    constant Bool intLessEqual = 2 <= 2
    constant Bool intGreaterEqual = 2 >= 3
    constant Bool intEqual = 2 == 2
    constant Bool intNotEqual = 2 != 3
    constant Bool floatLess = 1.5 < 2.5
    constant Bool floatGreaterEqual = 2.5 >= 2.5
    constant Bool boolEqual = true == false
    constant Bool boolNotEqual = true != false
  )");
  for (const auto& [name, expected] :
       std::initializer_list<std::pair<std::string_view, bool>>{
           {"intLess", true},
           {"intLessFalse", false},
           {"intGreater", true},
           {"intLessEqual", true},
           {"intGreaterEqual", false},
           {"intEqual", true},
           {"intNotEqual", true},
           {"floatLess", true},
           {"floatGreaterEqual", true},
           {"boolEqual", false},
           {"boolNotEqual", true}}) {
    const auto value = xsmp::getValue(expression(name));
    ASSERT_TRUE(value) << name;
    EXPECT_EQ(value->type, xsmp::PTK::Bool) << name;
    EXPECT_EQ(value->boolean, expected) << name;
  }
}

TEST_F(SolverExpressions, CircularConstantReferencesAreLinkedAndReturnNoValue) {
  parse("constant Int32 A = B constant Int32 B = A constant Int32 Self = Self");
  const auto* reference =
      cursiv::ast_cast<xsmp::NamedElementReference>(expression("A"));
  ASSERT_NE(reference, nullptr);
  ASSERT_TRUE(reference->value);
  ASSERT_NE(reference->value.get<xsmp::Constant>(), nullptr);
  EXPECT_EQ(reference->value.get<xsmp::Constant>()->name, "B");
  for (auto name : {"A", "B", "Self"})
    EXPECT_FALSE(xsmp::getValue(expression(name))) << name;
}

TEST_F(SolverExpressions, OctalLiteralUsesBaseEight) {
  parse("constant Int32 oct = 017");
  const auto value = xsmp::getValue(expression("oct"));
  ASSERT_TRUE(value);
  EXPECT_EQ(value->integral, 15u);
}

TEST_F(SolverExpressions, OutOfRangeChar8ConversionReturnsNoValue) {
  parse("constant Char8 c = 0x110000");
  EXPECT_FALSE(xsmp::getValueAs(expression("c"), xsmp::PTK::Char8));
}

TEST_F(SolverExpressions, DivisionByZeroDiagnostic) {
  parse("constant Int32 z = 1 / 0");
  std::vector<cursiv::Diagnostic> diagnostics;
  auto sink = [&](cursiv::Diagnostic&& diagnostic) {
    diagnostics.push_back(std::move(diagnostic));
  };
  cursiv::ValidationAcceptor accept(sink, *document, "xsmp");
  EXPECT_FALSE(xsmp::getValue(expression("z"), &accept));
  ASSERT_EQ(diagnostics.size(), 1u);
  EXPECT_EQ(diagnostics.front().message, "Division by zero.");
}

namespace {

std::vector<std::string> messagesOf(const cursiv::Document& document,
                                    const xsmp::Expression* expression) {
  std::vector<std::string> messages;
  auto sink = [&](cursiv::Diagnostic&& diagnostic) {
    messages.push_back(std::move(diagnostic.message));
  };
  cursiv::ValidationAcceptor accept(sink, document, "xsmp");
  const auto* constant = cursiv::ast_cast<xsmp::Constant>(expression->parent);
  if (!constant || !constant->type.get()) {
    ADD_FAILURE() << "A constant with a resolved type is expected.";
    return messages;
  }
  (void)xsmp::getValueAs(expression, *constant->type.get(), &accept);
  return messages;
}

}  // namespace

// As in C++, a decimal without `u` is signed, while another base may be
// unsigned.
TEST_F(SolverExpressions, DecimalLiteralsAreSigned) {
  parse(R"(
    constant Int64 negated = -4294967295
    constant Int32 lowest = -2147483648
    constant Int64 decimal = 2147483648
    constant Int64 hexadecimal = 0xFFFFFFFF
    constant Int64 wrapped = -0xFFFFFFFF
  )",
        true);
  const auto negated = xsmp::getValue(expression("negated"));
  ASSERT_TRUE(negated);
  EXPECT_EQ(asInt(*negated), -4294967295LL);
  EXPECT_EQ(negated->type, xsmp::PTK::Int64);
  EXPECT_TRUE(messagesOf(*document, expression("lowest")).empty());
  EXPECT_EQ(xsmp::getValue(expression("decimal"))->type, xsmp::PTK::Int64);
  EXPECT_EQ(xsmp::getValue(expression("hexadecimal"))->type, xsmp::PTK::UInt32);
  EXPECT_EQ(asInt(*xsmp::getValue(expression("wrapped"))), 1);
}

// As in C++, a small kind reads as Int32 in any operation.
TEST_F(SolverExpressions, SmallKindsArePromotedToInt32) {
  parse(R"(
    constant UInt8 byte = 200
    constant Int8 lowestByte = -128
    constant Int32 shifted = 1 / ((byte << 4) - 128)
    constant Int32 sum = 1000 + byte
    constant Int32 product = 256 * byte
    constant Int32 negated = -lowestByte
    constant Int32 complement = ~byte
    constant Int64 mixed = byte + 1L
    constant UInt32 mixedUnsigned = byte + 1U
  )",
        true);
  EXPECT_TRUE(messagesOf(*document, expression("shifted")).empty());
  EXPECT_EQ(asInt(*xsmp::getValue(expression("sum"))), 1200);
  EXPECT_EQ(asInt(*xsmp::getValue(expression("product"))), 51200);
  EXPECT_EQ(asInt(*xsmp::getValue(expression("negated"))), 128);
  EXPECT_EQ(asInt(*xsmp::getValue(expression("complement"))), -201);
  EXPECT_EQ(xsmp::getValue(expression("mixed"))->type, xsmp::PTK::Int64);
  EXPECT_EQ(xsmp::getValue(expression("mixedUnsigned"))->type,
            xsmp::PTK::UInt32);
}

TEST_F(SolverExpressions, WhatHasNoValueIsReported) {
  parse(R"(
    constant Int32 first = second
    constant Int32 second = first
    constant Int32 behind = first
    constant Int32 itself = itself
    constant Int32 collection = {1, 2}
    constant Int32 pointer = nullptr
    constant Int32 broken = 1 / 0
    constant Int32 reader = broken
  )",
        true);
  const std::vector<std::string> circular{
      "The value of this Constant is circular."};
  const std::vector<std::string> noValue{
      "A simple constant value is expected."};
  EXPECT_EQ(messagesOf(*document, expression("first")), circular);
  EXPECT_EQ(messagesOf(*document, expression("behind")), circular);
  EXPECT_EQ(messagesOf(*document, expression("itself")), circular);
  EXPECT_EQ(messagesOf(*document, expression("collection")), noValue);
  EXPECT_EQ(messagesOf(*document, expression("pointer")),
            std::vector<std::string>{xsmp::nullptrNeedsAPointer});
  // A Constant that fails on its own is reported where it is declared.
  EXPECT_TRUE(messagesOf(*document, expression("reader")).empty());
}

// Without a memo, each Constant would read the previous one twice over: 2^40.
TEST_F(SolverExpressions, ConstantsThatReadEachOtherTwiceStayLinear) {
  std::string body = "constant Int64 c0 = 1 ";
  for (auto i = 1; i <= 40; ++i)
    body += "constant Int64 c" + std::to_string(i) + " = c"
            + std::to_string(i - 1) + " + c" + std::to_string(i - 1) + " ";
  parse(body, true);
  EXPECT_EQ(asInt(*xsmp::getValue(expression("c40"))), 1LL << 40);
}

// An operand that cannot change the value of `&&` or `||` is only checked: a
// value it would fault on is no error, a type it cannot take still is.
TEST_F(SolverExpressions, LogicalOperatorsSkipTheOperandTheyDoNotNeed) {
  parse(R"(
    constant Bool a = false && (1 / 0 == 0)
    constant Bool b = true || (1 / 0 == 0)
    constant Bool c = false && (1 / (1 - 1) == 0)
    constant Bool d = true || (1 << -1)
    constant Bool e = false && (0 % 0)
    constant Bool f = true || (false && (1 / 0))
    constant Bool g = false && ("text" - 1)
    constant Bool h = true || $unknown(1)
    constant Bool i = true || $sqrt("text")
    constant Bool j = false && (true || ("text" / 2))
    constant Bool k = true && (1 / 0)
    constant Bool l = false || (1 / 0)
  )",
        true);
  for (const auto& [name, expected] : {std::pair{"a", false},
                                       {"b", true},
                                       {"c", false},
                                       {"d", true},
                                       {"e", false},
                                       {"f", true}}) {
    EXPECT_TRUE(messagesOf(*document, expression(name)).empty()) << name;
    const auto value = xsmp::getValue(expression(name));
    ASSERT_TRUE(value) << name;
    EXPECT_EQ(value->boolean, expected) << name;
  }
  for (const auto& [name, message] : {std::pair{"g", "Binary operator"},
                                      {"h", "Unknown built-in function"},
                                      {"i", "cannot be converted"},
                                      {"j", "Binary operator"}}) {
    const auto messages = messagesOf(*document, expression(name));
    EXPECT_TRUE(std::ranges::any_of(messages, [&](const std::string& text) {
      return text.find(message) != std::string::npos;
    })) << name;
  }
  for (const auto* name : {"k", "l"}) {
    EXPECT_FALSE(xsmp::getValue(expression(name))) << name;
    EXPECT_EQ(messagesOf(*document, expression(name)),
              std::vector<std::string>{"Division by zero."})
        << name;
  }
}

TEST_F(SolverExpressions, FloatingValuesShallBeFinite) {
  parse(R"(
    constant Float32 wide = 1.0e40
    constant Float64 huge = 1.0e400
    constant Float64 infinite = 1.0 / 0.0
    constant Float32 fits = 1.0e30
  )",
        true);
  EXPECT_EQ(messagesOf(*document, expression("wide")),
            std::vector<std::string>{"Conversion overflow for type Float32."});
  const std::vector<std::string> overflow{
      "Conversion overflow for type Float64."};
  EXPECT_EQ(messagesOf(*document, expression("huge")), overflow);
  EXPECT_EQ(messagesOf(*document, expression("infinite")), overflow);
  EXPECT_TRUE(messagesOf(*document, expression("fits")).empty());
}

TEST_F(SolverExpressions, NegativeShiftDiagnostic) {
  parse("constant Int32 s = 1 << -1");
  std::vector<cursiv::Diagnostic> diagnostics;
  auto sink = [&](cursiv::Diagnostic&& diagnostic) {
    diagnostics.push_back(std::move(diagnostic));
  };
  cursiv::ValidationAcceptor accept(sink, *document, "xsmp");
  EXPECT_FALSE(xsmp::getValue(expression("s"), &accept));
  ASSERT_EQ(diagnostics.size(), 1u);
  EXPECT_EQ(diagnostics.front().message, "Shift amount must be non-negative.");
}

TEST(Solver, DatesAreIsoDateTimes) {
  for (auto text :
       {"2020-01-01T00:00:00Z", "2020-01-01T00:00:00", "2020-02-29T00:00:00Z",
        "2020-01-01T24:00:00Z", "2020-01-01T00:00:00.5+14:00", "2020-01-01",
        "2020-01-01 01:00:00Z", "2020-01-01t01:00:00z",
        "2020-01-01T00:00:00,1Z"})
    EXPECT_TRUE(xsmp::isValidDateTime(text)) << text;
  // Any date for a document, but a DateTime only holds 64 bits of nanoseconds.
  EXPECT_FALSE(xsmp::isValidDateTime("-0044-03-15T12:00:00Z"));
  EXPECT_TRUE(xsmp::parseDateTimeMilliseconds("-0044-03-15T12:00:00Z"));
  for (auto text :
       {"2020", "2020-05", "2020/01/02", "1/2/2020", "2021-02-29T00:00:00Z",
        "2020-02-30T00:00:00Z", "2020-99-99T00:00:00Z", "2020-01-01T25:00:00Z",
        "2020-01-01T23:00:60Z", "2020-01-01T00:00:00+25:00",
        "2020-01-01T00:00:00+14:01", "2020-01-01T00:00:00+0200",
        "2020-01-01T24:00:00.1Z", "Fri, 21 Nov 1997 09:55:06 GMT",
        "October 22, 2022 09:07:44 GMT", "not a date"})
    EXPECT_FALSE(xsmp::isValidDateTime(text)) << text;
}

TEST_F(SolverExpressions, FloatToIntegerRefusesFractionsAndOutOfRangeValues) {
  parse(R"(
    constant Float64 fraction = 1.5
    constant Float64 whole = 3.0
    constant Float64 enormous = 1e300
    constant Float64 negative = -2.0
    constant Float64 tooWide = 258.0
    constant Float64 edge = 255.0
  )");
  EXPECT_FALSE(xsmp::getValueAs(expression("fraction"), xsmp::PTK::Int32));
  const auto whole = xsmp::getValueAs(expression("whole"), xsmp::PTK::Int32);
  ASSERT_TRUE(whole);
  EXPECT_EQ(whole->integral, 3u);
  EXPECT_FALSE(xsmp::getValueAs(expression("enormous"), xsmp::PTK::Int64));
  EXPECT_FALSE(xsmp::getValueAs(expression("negative"), xsmp::PTK::UInt64));
  EXPECT_FALSE(xsmp::getValueAs(expression("tooWide"), xsmp::PTK::UInt8));
  const auto edge = xsmp::getValueAs(expression("edge"), xsmp::PTK::UInt8);
  ASSERT_TRUE(edge);
  EXPECT_EQ(edge->integral, 255u);
}

TEST_F(SolverExpressions,
       TimeLiteralConversionKeepsTheBaseAndUsesInt64Promotion) {
  parse(
      "constant Int64 octalTime = 017ms constant Int64 smallTime = "
      "42us constant Int64 octalSeconds = 010s constant Int64 zero = 0s");
  const auto octalTime = xsmp::getValue(expression("octalTime"));
  const auto smallTime = xsmp::getValue(expression("smallTime"));
  ASSERT_TRUE(octalTime);
  EXPECT_EQ(octalTime->integral, 15'000'000u);
  EXPECT_EQ(octalTime->type, xsmp::PTK::Int64);
  EXPECT_EQ(xsmp::getValue(expression("octalSeconds"))->integral,
            8'000'000'000u);
  EXPECT_EQ(xsmp::getValue(expression("zero"))->integral, 0u);
  ASSERT_TRUE(smallTime);
  EXPECT_EQ(smallTime->integral, 42'000u);
  EXPECT_EQ(smallTime->type, xsmp::PTK::Int64);
}

TEST_F(SolverExpressions,
       ArbitrarilyLargeIntegerSpellingsCannotWrapBeforeRangeChecking) {
  parse(
      "constant UInt64 huge = 340282366920938463463374607431768211457 "
      "constant UInt64 hugeTime = 340282366920938463463374607431768211457s");
  for (auto name : {"huge", "hugeTime"}) {
    std::vector<cursiv::Diagnostic> diagnostics;
    auto sink = [&](cursiv::Diagnostic&& diagnostic) {
      diagnostics.push_back(std::move(diagnostic));
    };
    cursiv::ValidationAcceptor accept(sink, *document, "xsmp");
    EXPECT_FALSE(xsmp::getValue(expression(name), &accept));
    ASSERT_EQ(diagnostics.size(), 1u);
    EXPECT_EQ(diagnostics.front().message,
              "Conversion overflow for type UInt64.");
  }
}

TEST_F(SolverExpressions, FloatingLiteralUnderflowAndOverflowKeepIeeeValues) {
  parse("constant Float64 large = 1e999 constant Float64 tiny = 1e-999");
  const auto large = xsmp::getValue(expression("large"));
  const auto tiny = xsmp::getValue(expression("tiny"));
  ASSERT_TRUE(large);
  EXPECT_TRUE(std::isinf(large->floating));
  ASSERT_TRUE(tiny);
  EXPECT_DOUBLE_EQ(tiny->floating, 0.0);
}

TEST(Solver, NumericTypesExtendingEachOtherStayUnlinked) {
  auto ws = xsmp::TestWorkspace::withShared<xsmp::XsmpSharedServices>();
  auto& shared = ws.shared<xsmp::XsmpSharedServices>();
  shared.projectManager = std::make_unique<xsmp::ProjectManager>(shared);
  ws.registerLanguage(xsmp::createXsmpcatServices(shared));
  const auto result = cursiv::test::validationHelper(
      ws.language("xsmpcat"), R"(catalogue Cycles namespace n {
    integer Self extends Self
    float A extends B integer B extends A
    struct K { constant Self x = 1 constant A y = 1.0 }
  })");
  ASSERT_TRUE(result.document->parseSucceeded());
  std::size_t numericTypes = 0;
  for (const auto* node :
       xsmp::descendantsOf(*result.document->parseResult.value))
    if (const auto* type = cursiv::ast_cast<xsmp::Type>(node);
        cursiv::ast_cast<xsmp::Integer>(type)
        || cursiv::ast_cast<xsmp::Float>(type)) {
      EXPECT_EQ(xsmp::getPTK(type), cursiv::ast_cast<xsmp::Float>(type)
                                        ? xsmp::PTK::Float64
                                        : xsmp::PTK::Int32);
      ++numericTypes;
    }
  EXPECT_EQ(numericTypes, 3u);
  EXPECT_FALSE(result.document->diagnostics.empty());
}

TEST_F(SolverExpressions, Char8LengthCountsUtf8Bytes) {
  parse(
      "constant Char8 ascii = 'a' constant Char8 accented = 'é' constant Char8 "
      "emoji = '\\U0001F600'");
  for (const auto& [name, valid] :
       std::initializer_list<std::pair<std::string_view, bool>>{
           {"ascii", true}, {"accented", false}, {"emoji", false}}) {
    std::vector<cursiv::Diagnostic> diagnostics;
    auto sink = [&](cursiv::Diagnostic&& d) {
      diagnostics.push_back(std::move(d));
    };
    cursiv::ValidationAcceptor accept(sink, *document, "xsmp");
    ASSERT_TRUE(xsmp::getValueAs(expression(name), xsmp::PTK::Char8, &accept))
        << name;
    if (valid) {
      EXPECT_TRUE(diagnostics.empty()) << name;
    } else {
      ASSERT_EQ(diagnostics.size(), 1u) << name;
      EXPECT_EQ(diagnostics[0].message,
                "A 'Char8' shall contain exactly one character.")
          << name;
    }
  }
}

TEST_F(SolverExpressions, UnsupportedOperatorsAndBuiltinArgumentsAreDiagnosed) {
  parse(R"(
    constant Bool logical = "x" || true
    constant Float64 unknown = $unknown("x")
    constant Float64 missing = $unknown()
    constant Float64 argument = $sqrt("x")
  )");
  const std::pair<std::string_view, std::vector<std::string_view>> cases[] = {
      {"logical",
       {"Binary operator '||' is not supported for operands of type 'String8' "
        "and 'Bool'."}},
      {"unknown", {"Unknown built-in function 'unknown'."}},
      {"missing",
       {"Missing argument.", "Unknown built-in function 'unknown'."}},
      {"argument", {"String8 cannot be converted to Float64."}}};
  for (const auto& [name, expected] : cases) {
    std::vector<cursiv::Diagnostic> diagnostics;
    auto sink = [&](cursiv::Diagnostic&& d) {
      diagnostics.push_back(std::move(d));
    };
    cursiv::ValidationAcceptor accept(sink, *document, "xsmp");
    EXPECT_FALSE(xsmp::getValue(expression(name), &accept)) << name;
    ASSERT_EQ(diagnostics.size(), expected.size()) << name;
    for (std::size_t i = 0; i < expected.size(); ++i)
      EXPECT_EQ(diagnostics[i].message, expected[i]) << name;
  }
}

TEST(Solver,
     DirectTypeConversionsValidateBoundsIncludingUnsigned64AndUtf8Length) {
  auto ws = xsmp::TestWorkspace::withShared<xsmp::XsmpSharedServices>();
  auto& shared = ws.shared<xsmp::XsmpSharedServices>();
  shared.projectManager = std::make_unique<xsmp::ProjectManager>(shared);
  ws.registerLanguage(xsmp::createXsmpcatServices(shared));
  xsmp::loadXsmpcatBuiltins(shared);
  auto document = xsmp::parse(ws, "xsmpcat", R"(catalogue Bounds namespace n {
    integer Wide extends Smp.UInt64 in 0 ... 18446744073709551614UL
    float Limited in 0.1 ... 0.5
    string Text [4]
    struct K {
      constant Wide valid = 9223372036854775808UL
      constant Wide invalid = 18446744073709551615UL
      constant Limited floatInvalid = 0.7
      constant Text bmp = "éà"
      constant Text supplementary = "\U0001F600"
      constant Text emoji = "😀"
      constant Text tooLong = "\U0001F600x"
      constant Text accentedTooLong = "éàx"
    }
  })");
  ASSERT_TRUE(document->parseSucceeded());
  const auto* catalogue =
      cursiv::ast_cast<xsmp::Catalogue>(document->parseResult.value);
  ASSERT_NE(catalogue, nullptr);
  const std::pair<std::string_view, std::string_view> cases[] = {
      {"valid", ""},
      {"invalid",
       "Integral value shall be less than or equal to 18446744073709551614."},
      {"floatInvalid", "Float value shall be less than or equal to 0.5."},
      {"bmp", ""},
      {"supplementary", ""},
      {"emoji", ""},
      {"tooLong", "The string length exceeds the allowed length for its type."},
      {"accentedTooLong",
       "The string length exceeds the allowed length for its type."}};
  for (const auto& [name, expected] : cases) {
    const auto* expression = constantValue(*catalogue, name);
    ASSERT_NE(expression, nullptr);
    const auto* constant = cursiv::ast_cast<xsmp::Constant>(expression->parent);
    ASSERT_NE(constant, nullptr);
    ASSERT_NE(constant->type.get(), nullptr);
    std::vector<cursiv::Diagnostic> diagnostics;
    auto sink = [&](cursiv::Diagnostic&& d) {
      diagnostics.push_back(std::move(d));
    };
    cursiv::ValidationAcceptor accept(sink, *document, "xsmp");
    EXPECT_TRUE(xsmp::getValueAs(expression, *constant->type.get(), &accept))
        << name;
    ASSERT_EQ(diagnostics.size(), expected.empty() ? 0u : 1u) << name;
    if (!expected.empty()) {
      EXPECT_EQ(diagnostics[0].message, expected) << name;
      EXPECT_EQ(diagnostics[0].relatedInformation.size(), 1u) << name;
    }
  }
}

TEST(Solver,
     DateParseMillisecondsTruncatesFractionsBeforeNanosecondConversion) {
  EXPECT_EQ(xsmp::parseDateTimeMilliseconds("1969-12-31T23:59:59.999999999Z"),
            -1);
  EXPECT_EQ(xsmp::parseDateTimeMilliseconds("1970-01-01T00:00:00.000999999Z"),
            0);
  // A date without a zone reads as UTC, whatever the machine.
  EXPECT_EQ(xsmp::parseDateTimeMilliseconds("1970-01-01T00:00:01"), 1000);
}
