#include <gtest/gtest.h>

#include <algorithm>
#include <string>
#include <vector>
#include <xsmp/DeploymentSemantics.hpp>
#include <xsmp/ast.hpp>
#include <xsmp/utils/TreeUtils.hpp>

#include "core/validation/DeploymentSemanticsFixture.hpp"
#include "support/DeploymentWorld.hpp"

using DeploymentValueValidatorTest = DeploymentSemantics;

TEST_F(DeploymentValueValidatorTest,
       DeepFiniteArrayValuesKeepExpectedLeafTypes) {
  std::string types =
      "catalogue Nested namespace nested { public array A0 = Smp.Int32[1]";
  for (unsigned index = 1; index <= 70; ++index)
    types += " public array A" + std::to_string(index) + " = A"
             + std::to_string(index - 1) + "[1]";
  types += " public model M { field A70 values } }";
  add("src/nested.xsmpcat", std::move(types));
  const auto document =
      build("xsmpcfg", "configuration NestedInit /root: nested.M { values = "
                           + std::string(71, '[') + "1" + std::string(71, ']')
                           + " }");
  clean(*document);
  const xsmp::IntValue* leaf = nullptr;
  for (const auto* node : xsmp::descendantsOf(*document->parseResult.value))
    if (const auto* integer = cursiv::ast_cast<xsmp::IntValue>(node))
      leaf = integer;
  ASSERT_NE(leaf, nullptr);
  const auto* type = xsmp::expectedValueType(leaf);
  ASSERT_NE(type, nullptr);
  EXPECT_EQ(type->name, "Int32");
}

// A Char8 holds one byte: a character that takes several UTF-8 bytes is
// refused.
TEST_F(DeploymentValueValidatorTest, Char8LengthCountsUtf8Bytes) {
  add("src/unicode.xsmpcat",
      "catalogue Unicode namespace unicode { public model M { field Smp.Char8 "
      "c } }");
  clean(*build("xsmpcfg",
               "configuration AsciiConfig /root: unicode.M { c = 'a' }"));
  const auto accented = build(
      "xsmpcfg", "configuration AccentedChar /root: unicode.M { c = 'é' }");
  EXPECT_TRUE(has(*accented, "exactly one character"));
}

// A deployed string fits the length its type declares, counted in UTF-8 bytes.
TEST_F(DeploymentValueValidatorTest, StringValueFitsTheDeclaredLength) {
  add("src/strings.xsmpcat",
      "catalogue Strings namespace strings { public string Short[2] "
      "public model M { field Short s } }");
  clean(
      *build("xsmpcfg", "configuration Fits /root: strings.M { s = \"ab\" }"));
  const auto tooLong = build(
      "xsmpcfg", "configuration TooLong /root: strings.M { s = \"abc\" }");
  EXPECT_TRUE(has(*tooLong, "exceeds the allowed length"));
  const auto accented = build(
      "xsmpcfg", "configuration Accented /root: strings.M { s = \"aé\" }");
  EXPECT_TRUE(has(*accented, "exceeds the allowed length"));
}

// The size of a deployed array is a non-negative 64-bit integer. Another size
// gets no diagnostic from the deployment: the catalogue reports it.
TEST_F(DeploymentValueValidatorTest,
       ArraySizesOutsideTheUnsigned64RangeGetNoDiagnostic) {
  add("src/wide-array.xsmpcat", R"(catalogue WideArray namespace wide {
    public array Negative = Smp.Int32[-1e300]
    public array Minus = Smp.Int32[-1]
    public array Fraction = Smp.Int32[2.5]
    public array Wide = Smp.Int32[5000000000]
    public model M { field Negative a field Minus b field Fraction c field Wide d }
  })");
  const auto document = build(
      "xsmpcfg",
      "configuration Wide /root: wide.M { a = [1] b = [1] c = [1] d = [1] }");
  EXPECT_FALSE(has(*document, "shall not contain more than"));
  const auto tooMany = build(
      "xsmpcfg", "configuration Many /root: wide.M { b = [1, 2] d = [1, 2] }");
  EXPECT_FALSE(has(*tooMany, "shall not contain more than"));
}

TEST_F(DeploymentValueValidatorTest,
       ConfigurationInfersUnsuffixedNestedValueTypes) {
  auto document = build("xsmpcfg", R"(configuration Nominal
/arbitraryLabel: demo.Root {
 flag = true tiny = 2 ratio = 1.5 values = [1, 2]
 state = { count = 1, enabled = false, values = [1, 2] }
 child: demo.Child { inValue = 7 }
 include Other at child
})");
  clean(*document);
}

TEST_F(DeploymentValueValidatorTest,
       ConfigurationChecksValueKindsRangesAndCollectionStructure) {
  auto document = build("xsmpcfg", R"(configuration Invalid
/root: demo.Root {
 flag = 1i32 tiny = 8 ratio = 2.0f32 values = [1i32, 2i32, 3i32]
 state = { count = true, count = 1i32, missing = 1i32, values = [true] }
})");
  EXPECT_TRUE(has(*document, "compatible with type Smp.Bool"));
  EXPECT_TRUE(
      has(*document, "Integral value shall be less than or equal to 3"));
  EXPECT_TRUE(has(*document, "Float value shall be less than 2"));
  EXPECT_TRUE(has(*document, "shall not contain more than 2 item(s)"));
  EXPECT_TRUE(has(*document, "shall not be initialized more than once"));
  EXPECT_TRUE(has(*document, "structure field 'missing' does not exist"));
  EXPECT_TRUE(has(*document, "compatible with type Smp.Int32"));
}

TEST_F(DeploymentValueValidatorTest,
       UnsafeAndLegacyPathsCannotInferUnsuffixedNumericTypes) {
  auto document = build("xsmpcfg", R"(configuration Legacy
/root { unknown = 1 }
/typed: demo.Root { unsafe ratio = 1.5 state = { unsafe count = 1 } }
)");
  EXPECT_TRUE(has(*document, "write a suffix, as in 1i32"));
  EXPECT_TRUE(has(*document, "write a suffix, as in 1.0f64"));
}

TEST_F(DeploymentValueValidatorTest,
       AssemblyStructureInitializersUseTheirExpectedType) {
  add("src/nested-types.xsmpcat", R"(catalogue NestedTypes
namespace nested {
 public struct Inner { field Smp.Int32 count field Smp.Bool enabled }
 public array Inners = Inner[2]
 public struct State { field Inner inner field Inners values }
 public model Root { field State state }
})");
  auto document = build("xsmpasb", R"(assembly StructurePaths
Root: nested.Root {
 state = { inner = { count = 1, enabled = true }, values = [{count = 2}, {count = 3}] }
 state = { inner.count = 4, inner.enabled = false, values[0].count = 5 }
})");
  clean(*document);
  auto checked = false;
  for (const auto* node : xsmp::descendantsOf(*document->parseResult.value)) {
    if (const auto* number = cursiv::ast_cast<xsmp::IntValue>(node)) {
      const auto* type = xsmp::expectedValueType(number);
      ASSERT_NE(type, nullptr);
      EXPECT_EQ(type->name, "Int32");
      checked = true;
    }
  }
  EXPECT_TRUE(checked);
}

TEST_F(DeploymentValueValidatorTest,
       AssemblyStructureDesignatorsRejectUnknownPathsAndWrongKinds) {
  add("src/nested-types.xsmpcat", R"(catalogue NestedTypes
namespace nested {
 public struct Inner { field Smp.Int32 count }
 public struct State { field Inner inner }
 public model Root { field State state }
})");
  auto document = build("xsmpasb", R"(assembly InvalidStructures
Root: nested.Root { state = { inner.count = true, inner.missing = 1i32 } }
)");
  EXPECT_TRUE(has(*document, "compatible with type Smp.Int32"));
  EXPECT_TRUE(has(*document, "structure field path"));
  EXPECT_FALSE(has(*document, "current Component"));
}

TEST_F(DeploymentValueValidatorTest,
       DeploymentStringsAndCharsUseTheSharedEscapeConverter) {
  add("src/text-types.xsmpcat", R"(catalogue TextTypes namespace text {
 public string Label[32]
 public model Root { field Smp.Char8 grade field Label label }
})");
  auto document = build("xsmpcfg", R"(configuration Escapes
/root: text.Root { grade = '\n' label = "line\n\t\x41\u00e9" }
)");
  clean(*document);
  auto character = false, string = false;
  for (const auto* node : xsmp::descendantsOf(*document->parseResult.value)) {
    if (const auto* value = cursiv::ast_cast<xsmp::Char8Value>(node)) {
      EXPECT_EQ(value->value, "\n");
      character = true;
    }
    if (const auto* value = cursiv::ast_cast<xsmp::String8Value>(node)) {
      EXPECT_EQ(value->value, "line\n\tA\xc3\xa9");
      string = true;
    }
  }
  EXPECT_TRUE(character);
  EXPECT_TRUE(string);
}

// A Char8 is one byte, whichever way it is written.
TEST_F(DeploymentValueValidatorTest, ACharIsOneByteAsInCpp) {
  add("src/bytes.xsmpcat",
      "catalogue Bytes namespace bytes { public model M { field Smp.Char8 c "
      "field Smp.Char8 d field Smp.Char8 e } }");
  clean(*build("xsmpcfg",
               R"(configuration OneByte /root: bytes.M { c = '\xff' d = '\377'
e = '\x4' })"));
  for (const auto* text : {R"('\u00e9')", R"('\xC3\xA9')"})
    EXPECT_TRUE(
        has(*build("xsmpcfg", std::string("configuration TwoBytes /root: "
                                          "bytes.M { c = ")
                                  + text + " }"),
            "exactly one character"))
        << text;
}

// The length of a string type counts the bytes an escape names.
TEST_F(DeploymentValueValidatorTest, AStringLengthCountsEscapedBytes) {
  add("src/lengths.xsmpcat",
      "catalogue Lengths namespace lengths { public string Short[2] "
      "public model M { field Short s field Short t field Short u } }");
  clean(*build("xsmpcfg", R"(configuration Fit /root: lengths.M {
s = "\xFF\xFE" t = "\xC3\xA9" u = "é" })"));
  EXPECT_TRUE(has(*build("xsmpcfg", R"(configuration Over /root: lengths.M {
s = "\xFF\xFE\xFD" })"),
                  "exceeds the allowed length"));
}

// One message for one fault, wherever the literal stands.
TEST_F(DeploymentValueValidatorTest, ALiteralThatDoesNotDecodeIsReportedOnce) {
  add("src/undecoded.xsmpcat",
      "catalogue Undecoded namespace undecoded { public model M { field "
      "Smp.Char8 c field Smp.Duration d field Smp.DateTime t } }");
  const auto configuration =
      build("xsmpcfg", R"(configuration UndecodedC /root: undecoded.M {
  c = '\x'
  d = "\x"d
  t = "\x"dt
})");
  const auto assembly = build("xsmpasb", R"(assembly <Root = "\x"> UndecodedA
Instance: "undecoded.\x"
)");
  const auto errors = [](const cursiv::Document& document) {
    std::vector<std::string> found;
    for (const auto& diagnostic : document.diagnostics)
      if (diagnostic.severity == cursiv::DiagnosticSeverity::Error)
        found.push_back(diagnostic.message);
    std::ranges::sort(found);
    return found;
  };
  // A date, a duration and a template value stay as written: their format is
  // what fails.
  EXPECT_EQ(errors(*configuration),
            (std::vector<std::string>{
                R"('\x' needs a hexadecimal digit)",
                "The value shall be a valid DateTime (e.g: 1970-01-01T00:00:00Z).",
                "The value shall be a valid Duration (e.g: PT1S)."}));
  EXPECT_EQ(errors(*assembly),
            (std::vector<std::string>{
                R"('\x' needs a hexadecimal digit)",
                "A template value shall hold only letters, digits, '_' and "
                "'{parameter}'."}));
}

// Catalogue or deployment document: the same escapes, and the same warning.
TEST_F(DeploymentValueValidatorTest, AnUnknownEscapeIsAWarning) {
  add("src/escapes.xsmpcat",
      "catalogue Escapes2 namespace escapes { public model M { field Smp.Char8 "
      "c field Smp.Int32 i } }");
  const auto configuration = build("xsmpcfg", R"(configuration Warned
/root: escapes.M { c = '\q' /* \d */ })");
  const auto warned = [](const cursiv::Document& document) {
    std::vector<std::string> found;
    for (const auto& diagnostic : document.diagnostics)
      if (diagnostic.message.starts_with("Unknown escape"))
        found.push_back(diagnostic.message);
    return found;
  };
  EXPECT_EQ(warned(*configuration),
            std::vector<std::string>{R"(Unknown escape '\q')"});
  const auto schedule = build("xsmpsed", R"(schedule <Root = "R"> Warned2
task T { emit "C:\dir" }
)");
  EXPECT_EQ(warned(*schedule),
            std::vector<std::string>{R"(Unknown escape '\d')"});
  const auto assembly = build("xsmpasb", R"(assembly Warned3
Root: escapes.M { subscribe step -> "\8" }
)");
  EXPECT_EQ(warned(*assembly),
            std::vector<std::string>{R"(Unknown escape '\8')"});
}

TEST_F(DeploymentValueValidatorTest,
       UnsuffixedUInt64ValuesRetainTheEntirePrimitiveRange) {
  add("src/numeric-types.xsmpcat", R"(catalogue NumericTypes namespace numbers {
 public model Root { field Smp.UInt64 maximum field Smp.Int64 minimum }
})");
  auto document = build("xsmpcfg", R"(configuration NumericLimits
/root: numbers.Root { maximum = 18446744073709551615 minimum = -9223372036854775808 }
)");
  clean(*document);
  auto invalid = build("xsmpcfg", R"(configuration NumericOverflow
/root: numbers.Root { maximum = 18446744073709551616 minimum = -9223372036854775809 }
)");
  EXPECT_TRUE(invalid->parseSucceeded());
  EXPECT_TRUE(has(*invalid, "Conversion overflow for type UInt64"));
  EXPECT_TRUE(has(*invalid, "Conversion overflow for type Int64"));
}

TEST_F(DeploymentValueValidatorTest,
       TimesFollowTheXsdAndAnEventMayRepeatForever) {
  auto invalid = build("xsmpsed", R"(schedule <Root="root"> InvalidTimes
 epoch "2025-01-01T00:00:60Z" mission "2025-02-30T00:00:00Z"
 event Missing zulu "2025-01-01T24:01:00Z"
)");
  EXPECT_TRUE(has(*invalid, "EpochTime shall be a valid DateTime"));
  EXPECT_TRUE(has(*invalid, "MissionStart shall be a valid DateTime"));
  EXPECT_TRUE(has(*invalid, "ZuluTime shall be a valid DateTime"));
  auto valid = build("xsmpsed", R"(schedule <Root="root"> ValidTimes
 epoch "2025-01-01T24:00:00Z" mission "2025-02-28T00:00:00"
 task Main { emit "Boot" }
 event Main zulu "2025-01-01T12:34:56.123+02:30" cycle "PT1S" repeat -1
)");
  clean(*valid);
}

TEST_F(DeploymentValueValidatorTest,
       TimesAndLiteralsAreCheckedAgainstTheField) {
  add("src/typed.xsmpcat", R"(catalogue Typed namespace typed {
 public enum E { A = 0, B = 1 }
 public enum Other { C = 0 }
 public model M { field Smp.DateTime dt field Smp.Duration du field E e }
})");
  clean(*build("xsmpcfg", R"(configuration TypedGood /root: typed.M {
 dt = "2000-01-01T12:00:00Z"dt
 du = "PT1S"d
 e = typed.E.A
})"));
  clean(*build("xsmpcfg",
               "configuration TypedNumber /root: typed.M { e = 1 lit }"));
  const auto bad = build("xsmpcfg", R"(configuration TypedBad /root: typed.M {
 dt = "garbage"dt
 du = "garbage"d
 e = typed.Other.C
})");
  EXPECT_TRUE(has(*bad, "The value shall be a valid DateTime"));
  EXPECT_TRUE(has(*bad, "The value shall be a valid Duration"));
  EXPECT_TRUE(has(*bad, "Enumeration literal C is not of type typed.E."));
  EXPECT_TRUE(
      has(*build("xsmpcfg", R"(configuration TypedOutOfRange /root: typed.M {
 dt = "3000-01-01T00:00:00Z"dt
})"),
          "The value shall be a valid DateTime"));
  EXPECT_TRUE(has(
      *build("xsmpcfg", "configuration TypedLit /root: typed.M { e = 5 lit }"),
      "5 is not the value of a literal of type typed.E."));
}

// By position, a structure value reads the public fields that are not static,
// the base class first.
TEST_F(DeploymentValueValidatorTest, PositionalValuesFollowTheFieldsOfTheType) {
  add("src/positions.xsmpcat", R"(catalogue Positions namespace positions {
 public class Base { public field Smp.Bool x }
 public class Derived extends Base { public field Smp.Int32 y }
 public class Hidden {
   public field Smp.Int32 a
   private field Smp.Bool secret
   @Attributes.Static public field Smp.Bool shared
   public field Smp.Int32 b
 }
 public model M { field Derived derived field Hidden hidden }
})");
  clean(*build("xsmpcfg", R"(configuration PositionsGood /root: positions.M {
 derived = { true, 5 }
 hidden = { 1, 2 }
})"));
  EXPECT_TRUE(
      has(*build("xsmpcfg", R"(configuration PositionsMore /root: positions.M {
 hidden = { 1, 2, 3 }
})"),
          "shall not contain more values than the fields"));
}

TEST_F(DeploymentValueValidatorTest, CallsFollowTheSignatureOfTheOperation) {
  add("src/calls.xsmpcat", R"(catalogue Calls namespace calls {
 public model M {
   public def Smp.Int32 op(in Smp.Int32 a, in Smp.Int32 b)
   public def Smp.Bool answer named(in Smp.Int32 a, in Smp.Int32 b = 1)
   public def void nothing()
 }
})");
  const auto call = [&](std::string name, std::string_view text) {
    return build("xsmpasb", "assembly " + name + " R: calls.M { call "
                                + std::string(text) + " }\n");
  };
  clean(*call("CallAll", "op(a = 1, b = 2)"));
  clean(*call("CallReturn", "op(return = 1, a = 2, b = 3)"));
  clean(*call("CallReturnLast", "op(a = 2, b = 3, return = 1)"));
  clean(*call("CallNamed", "named(answer = true, a = 1)"));
  EXPECT_TRUE(has(*call("CallMissing", "op(a = 1)"),
                  "The parameter 'b' of operation op needs a value."));
  EXPECT_TRUE(has(*call("CallOrder", "op(b = 1, a = 2)"),
                  "The parameters shall come in the order of operation op."));
  EXPECT_TRUE(has(*call("CallReturnType", "op(return = true, a = 1, b = 2)"),
                  "compatible with type Smp.Int32"));
  EXPECT_TRUE(has(*call("CallNoReturn", "nothing(return = 1)"),
                  "shall resolve to a Parameter of operation nothing"));
  const auto repeated = call("CallRepeated", "op(a = 1, a = 2, b = 3)");
  EXPECT_TRUE(has(*repeated, "Duplicated parameter name."));
  EXPECT_FALSE(has(*repeated, "shall come in the order"));
}

TEST_F(DeploymentValueValidatorTest,
       NonFiniteFloat32OnlyReportsTheConversionOverflow) {
  const auto document =
      build("xsmpcfg",
            "configuration NonFinite /Root: demo.Root { ratio = 1e999f32 }");
  ASSERT_EQ(document->diagnostics.size(), 1u);
  EXPECT_EQ(document->diagnostics[0].message,
            "Conversion overflow for type Float32.");
}

TEST_F(DeploymentValueValidatorTest,
       PrimitiveBoundsUseOnlyTheContextualConversions) {
  add("src/scalars.xsmpcat",
      R"(catalogue Scalars namespace scalars { public model Root {
    field Smp.Int8 a field Smp.Int16 b field Smp.Int32 c
    field Smp.UInt8 d field Smp.UInt16 e field Smp.UInt32 f
    field Smp.Float32 finite
  } })");
  const auto document =
      build("xsmpcfg", R"(configuration Bounds /Root: scalars.Root {
    a = 128i8 b = 32768i16 c = 2147483648i32
    d = 256u8 e = 65536u16 f = 4294967296u32
    finite = 3.4028235e38f32
  })");
  // A Float32 is rounded before its finiteness is checked: the last number is
  // above the largest float but rounds to it, so it is accepted.
  ASSERT_EQ(document->diagnostics.size(), 6u);
  for (const auto kind :
       {"Int8", "Int16", "Int32", "UInt8", "UInt16", "UInt32"})
    EXPECT_TRUE(has(*document,
                    "Conversion overflow for type " + std::string(kind) + "."));
  EXPECT_FALSE(has(*document, "Value is out of range."));
  EXPECT_FALSE(has(*document, "Float32"));
}

TEST(DeploymentValueValidator,
     UnsafeNumericValuesReportDuplicatesAndAnEventMayRepeatForever) {
  xsmp::DeploymentWorld world;
  constexpr std::string_view source =
      R"(schedule <Root = "platform", Root = "other"> Plan
    task Main { call unsafe reset(value = 128i8, value = 1u8) }
    event Main simulation "PT1S" cycle "PT1S" repeat -1
  )";
  auto doc = world.build("xsmpsed", std::string(source));
  ASSERT_EQ(doc->diagnostics.size(), 2u);
  const auto expectError = [&](std::string_view message, std::string_view token,
                               std::size_t from = 0) {
    const auto issue = std::ranges::find_if(
        doc->diagnostics, [&](const auto& d) { return d.message == message; });
    ASSERT_NE(issue, doc->diagnostics.end()) << message;
    const auto begin = source.find(token, from);
    ASSERT_NE(begin, std::string_view::npos);
    EXPECT_EQ(static_cast<int>(issue->severity), 1);
    EXPECT_EQ(issue->range.begin, begin);
    EXPECT_EQ(issue->range.end, begin + token.size());
  };
  // An unsafe unresolved path supplies no target type against which the
  // suffixed numeric value could be range-checked.
  expectError("Duplicated template argument name.", "Root",
              source.find("Root") + 4);
  expectError("Duplicated parameter name.", "value = 1u8");
}

// In a structure value, a value with no field takes the next one still free,
// and is checked against that field, in an assembly as in a configuration.
TEST_F(DeploymentValueValidatorTest,
       PositionalStructureValuesAreCheckedInAnAssembly) {
  clean(*build("xsmpasb", R"(assembly Positional
Root: demo.Root { state = { 1i32, true, [1i32, 2i32] } })"));
  const auto wrong = build("xsmpasb", R"(assembly WrongKind
Root: demo.Root { state = { true, 1i32 } })");
  EXPECT_TRUE(has(*wrong, "compatible with type Smp.Int32"));
  EXPECT_TRUE(has(*wrong, "compatible with type Smp.Bool"));
  const auto many = build("xsmpasb", R"(assembly TooMany
Root: demo.Root { state = { 1i32, true, [1i32, 2i32], 4i32 } })");
  EXPECT_TRUE(has(*many, "more values than the fields"));
  clean(*build("xsmpasb", R"(assembly Mixed
Root: demo.Root { state = { enabled = true, 1i32 } })"));
}
