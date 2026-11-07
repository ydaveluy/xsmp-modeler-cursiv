#include <gtest/gtest.h>

#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
#include <xsmp/Builtins.hpp>
#include <xsmp/Traversal.hpp>
#include <xsmp/solver/Solver.hpp>
#include <xsmp/utils/XsmpUtils.hpp>

#include "support/DiagnosticTestSupport.hpp"
#include "support/EscapeCases.hpp"
#include "support/LspTestSupport.hpp"

namespace {

cursiv::test::ValidationResult validate(xsmp::TestWorkspace& ws,
                                        std::string text) {
  return cursiv::test::validationHelper(ws.language("xsmpcat"),
                                        std::move(text));
}

xsmp::TestWorkspace workspaceWithBuiltins() {
  auto ws = xsmp::xsmpcatWorkspace();
  xsmp::loadXsmpcatBuiltins(ws.shared<xsmp::XsmpSharedServices>());
  return ws;
}

}  // namespace

// A diagnostic on a doc tag covers the text of that tag, wherever the comment
// stands in the document and whatever its line ends are.
TEST(XsmpcatValidator, DocTagRangesAreOffsetsInTheDocument) {
  auto ws = xsmp::xsmpcatWorkspace();
  const std::string text =
      "catalogue c\r\nnamespace n {\r\n\r\n"
      "  /**\r\n   * A description.\r\n   * @uuid not-a-uuid\r\n   */\r\n"
      "  struct S {}\n}\n";
  auto result = validate(ws, text);
  const auto& diagnostics = result.document->diagnostics;
  const auto found = std::ranges::find(diagnostics, "The UUID is invalid.",
                                       &cursiv::Diagnostic::message);
  ASSERT_NE(found, diagnostics.end());
  EXPECT_EQ(
      text.substr(found->range.begin, found->range.end - found->range.begin),
      "not-a-uuid");
}

// A UUID is a number: its case does not count, and it counts only as a `@uuid`.
TEST(XsmpcatValidator, DuplicatedUuidsIgnoreCaseAndMentions) {
  auto ws = workspaceWithBuiltins();
  auto result = validate(ws, R"(catalogue c
namespace n {
  /** @uuid 0000000a-0000-0000-0000-000000000000 */
  struct A {}
  /** @uuid 0000000A-0000-0000-0000-000000000000 */
  struct B {}
  /** Not 0000000b-0000-0000-0000-000000000000.
   * @uuid 0000000c-0000-0000-0000-000000000000 */
  struct C {}
  /** @uuid 0000000b-0000-0000-0000-000000000000 */
  struct D {}
}
)");
  const auto& diagnostics = result.document->diagnostics;
  EXPECT_EQ(std::ranges::count(diagnostics, "Duplicated UUID.",
                               &cursiv::Diagnostic::message),
            2);
}

// The text of a comment says which documents to read: a UUID on the next line
// of its tag is found, and so is the UUID of a built-in.
TEST(XsmpcatValidator, DuplicatedUuidsAreFoundWhereverTheyAreWritten) {
  auto ws = workspaceWithBuiltins();
  auto result = validate(ws, R"(catalogue c
namespace n {
  /** @uuid 0000000a-0000-0000-0000-000000000000 */
  struct A {}
  /**
   * @uuid
   *   0000000A-0000-0000-0000-000000000000
   */
  struct B {}
  /** @uuid 8596d697-fb84-41ce-a685-6912006ed666 */
  struct C {}
  /** @uuid 0000000c-0000-0000-0000-000000000000 */
  struct D {}
}
)");
  const auto& diagnostics = result.document->diagnostics;
  EXPECT_EQ(std::ranges::count(diagnostics, "Duplicated UUID.",
                               &cursiv::Diagnostic::message),
            3);
}

// A comment is parsed from its tag: what stands before does not hide the tag.
TEST(XsmpcatValidator, ATagIsFoundAfterWhatOnlyLooksLikeIt) {
  auto ws = xsmp::xsmpcatWorkspace();
  const std::string text =
      "catalogue c\nnamespace n {\n"
      "  /**\n   * See @uuid in {@uuid this} text.\n   * @uuids many\n"
      "   * @uuid not-a-uuid\n   * @since 1\n   */\n"
      "  struct S {}\n}\n";
  auto result = validate(ws, text);
  const auto& diagnostics = result.document->diagnostics;
  const auto found = std::ranges::find(diagnostics, "The UUID is invalid.",
                                       &cursiv::Diagnostic::message);
  ASSERT_NE(found, diagnostics.end());
  EXPECT_EQ(
      text.substr(found->range.begin, found->range.end - found->range.begin),
      "not-a-uuid");
}

TEST(XsmpcatValidator, AttachedFieldMayHaveADerivedType) {
  auto ws = workspaceWithBuiltins();
  auto result = validate(ws, R"(catalogue c
namespace n {
  class Base {}
  class Derived extends Base {}
  class Other {}
  class Owner {
    field Derived d
    property Base good -> d
    property Other bad -> d
    property Derived exact -> d
  }
}
)");
  EXPECT_EQ(std::ranges::count(
                result.document->diagnostics,
                "The Type of the AttachedField shall match the Type of the "
                "Property.",
                &cursiv::Diagnostic::message),
            1);
}

TEST(XsmpcatValidator, ArraySizeShallBePositive) {
  auto ws = workspaceWithBuiltins();
  auto result = validate(ws, R"(catalogue c
namespace n {
  array Empty = Smp.Int32[0]
  array Good = Smp.Int32[1]
  array Text = Smp.Int32["a"]
}
)");
  const auto& diagnostics = result.document->diagnostics;
  EXPECT_EQ(std::ranges::count(diagnostics,
                               "The Array size shall be a positive number.",
                               &cursiv::Diagnostic::message),
            1);
  cursiv::test::expectError(
      result, cursiv::test::contains("String8 cannot be converted to Int64."));
  cursiv::test::expectNoIssues(
      result, {.messageMatches = cursiv::test::contains("Missing Array size")});
}

TEST(XsmpcatValidator, AttributeTypeShallHaveADefaultValue) {
  auto ws = workspaceWithBuiltins();
  cursiv::test::expectError(
      validate(ws,
               "catalogue c\nnamespace n { attribute "
               "Smp.Bool A }\n"),
      cursiv::test::contains("An AttributeType shall have a default value."));
  auto ws2 = workspaceWithBuiltins();
  cursiv::test::expectNoIssues(
      validate(ws2,
               "catalogue c\nnamespace n { attribute "
               "Smp.Bool A = true }\n"),
      {.messageMatches = cursiv::test::contains("default value")});
}

// A class holds what its base holds, so its fields cannot lead back to it
// through a derived class either.
TEST(XsmpcatValidator, RecursiveTypeFollowsTheBaseOfAClass) {
  auto ws = workspaceWithBuiltins();
  cursiv::test::expectError(validate(ws, R"(catalogue c
namespace n {
  class Upper { field Lower x }
  class Lower extends Upper {}
}
)"),
                            cursiv::test::contains("Recursive Field Type."));
  auto ws2 = workspaceWithBuiltins();
  cursiv::test::expectNoIssues(
      validate(ws2, R"(catalogue c
namespace n {
  class Upper {}
  class Lower extends Upper { field Upper x }
}
)"),
      {.messageMatches = cursiv::test::contains("Recursive")});
}

TEST(XsmpcatValidator, ProtectedConstantIsVisibleFromDerivedTypesOnly) {
  auto ws = workspaceWithBuiltins();
  auto result = validate(ws, R"(catalogue c
namespace n {
  public class A { protected constant Smp.Int32 k = 1 }
  public class Derived extends A { constant Smp.Int32 good = n.A.k }
  public class Unrelated { constant Smp.Int32 bad = n.A.k }
}
)");
  EXPECT_EQ(std::ranges::count(result.document->diagnostics,
                               "The Constant is not visible.",
                               &cursiv::Diagnostic::message),
            1);
}

TEST(XsmpcatValidator, MultiplicityBoundThatIsNoNumberIsReported) {
  auto ws = workspaceWithBuiltins();
  auto result = validate(ws, R"(catalogue c
namespace n {
  public model M {
    reference Smp.IObject["a"] text
    reference Smp.IObject[1.5] fraction
    container Smp.IObject[99999999999999999999] huge
    reference Smp.IObject[1 ... "x"] upper
  }
}
)");
  cursiv::test::expectError(
      result, cursiv::test::contains("String8 cannot be converted to Int64."));
  cursiv::test::expectError(
      result, cursiv::test::contains("Float64 cannot be converted to Int64."));
  cursiv::test::expectError(
      result, cursiv::test::contains("Conversion overflow for type UInt64."));
}

std::vector<std::string> errorsOf(const auto& result) {
  std::vector<std::string> errors;
  for (const auto& diagnostic : result.document->diagnostics)
    if (diagnostic.severity == cursiv::DiagnosticSeverity::Error)
      errors.push_back(diagnostic.message);
  return errors;
}

// One line of the catalogue per case, and whether its `nullptr` is valid: it is
// where the generated C++ type is a pointer.
TEST(XsmpcatValidator, NullptrIsValidWhereTheCppTypeIsAPointer) {
  constexpr std::pair<std::string_view, bool> cases[] = {
      // A pointer: `T*`, whatever the type.
      {"interface A1 { def void f(out Smp.Int32 p = nullptr) }", true},
      {"interface A2 { def void f(inout Smp.Int32 p = nullptr) }", true},
      {"interface A3 { def void f(inout Smp.Duration p = nullptr) }", true},
      {"interface A4 { def void f(out Smp.String8 p = nullptr) }", true},
      {"interface A5 { def void f(out Name p = nullptr) }", true},
      {"interface A6 { def void f(inout Point p = nullptr) }", true},
      {"interface A7 { def void f(inout Mode p = nullptr) }", true},
      {"interface A8 { def void f(out Pair p = nullptr) }", true},
      {"interface A9 { def void f(out Handle p = nullptr) }", true},
      {"interface B1 { def void f(out Target p = nullptr) }", true},
      {"interface B2 { def void f(inout Target p = nullptr) }", true},
      {"interface B3 { def void f(out Unit p = nullptr) }", true},
      {"interface B4 { def void f(@Attributes.ByPointer in Smp.Int32 p = "
       "nullptr) }",
       true},
      {"interface B5 { def void f(@Attributes.ByPointer Smp.Int32 p = nullptr) "
       "}",
       true},
      {"interface B6 { def void f(@Attributes.ByPointer in Target p = nullptr) "
       "}",
       true},
      {"interface B7 { def void f(@Attributes.Const @Attributes.ByPointer in "
       "Point p = nullptr) }",
       true},
      {"interface B8 { def void f(out Smp.Int32 p = (nullptr)) }", true},
      // A pointer by itself: `T*`, also through a const reference.
      {"interface C3 { def void f(in Ref p = nullptr) }", true},
      {"interface C4 { def void f(out Ref p = nullptr) }", true},
      {"struct C6 { field Ref f = nullptr }", true},
      {"struct D2 { field Nullable f = {.text = \"\", .ref = nullptr} }", true},
      {"interface G8 { def void f(@Attributes.ByReference in Ref p = nullptr) "
       "}",
       true},
      // A String8 stands for a text, as in the other XSMP tools.
      {"interface C1 { def void f(in Smp.String8 p = nullptr) }", false},
      {"interface C2 { def void f(Smp.String8 p = nullptr) }", false},
      {"struct C5 { field Smp.String8 f = nullptr }", false},
      {"struct D1 { field Texts f = {nullptr, \"a\"} }", false},
      {"struct D5 { field Nullable f = {.text = nullptr} }", false},
      // A NativeType is the platform's to define.
      {"interface D3 { def void f(in Handle p = nullptr) }", true},
      {"struct D4 { field Handle f = nullptr }", true},
      // A value.
      {"interface E1 { def void f(in Smp.Int32 p = nullptr) }", false},
      {"interface E2 { def void f(Smp.Int32 p = nullptr) }", false},
      {"interface E3 { def void f(in Smp.Bool p = nullptr) }", false},
      {"interface E4 { def void f(in Smp.Char8 p = nullptr) }", false},
      {"interface E5 { def void f(in Smp.Float64 p = nullptr) }", false},
      {"interface E6 { def void f(in Smp.Duration p = nullptr) }", false},
      {"interface E7 { def void f(in Mode p = nullptr) }", false},
      {"interface E8 { def void f(in Name p = nullptr) }", false},
      {"interface E9 { def void f(in Pair p = nullptr) }", false},
      {"interface F1 { def void f(in Point p = nullptr) }", false},
      {"interface F2 { def void f(in Thing p = nullptr) }", false},
      {"interface F3 { def void f(@Attributes.ByPointer(false) out Smp.Int32 p "
       "= nullptr) }",
       false},
      {"interface F4 { def void f(in Smp.Int32 p = (nullptr)) }", false},
      // A C++ reference.
      {"interface G1 { def void f(in Target p = nullptr) }", false},
      {"interface G2 { def void f(Target p = nullptr) }", false},
      {"interface G3 { def void f(in Unit p = nullptr) }", false},
      {"interface G4 { def void f(@Attributes.ByReference out Target p = "
       "nullptr) }",
       false},
      {"interface G5 { def void f(@Attributes.ByReference inout Smp.Int32 p = "
       "nullptr) }",
       false},
      {"interface G6 { def void f(@Attributes.ByReference in Smp.String8 p = "
       "nullptr) }",
       false},
      {"interface G7 { def void f(@Attributes.Const @Attributes.ByReference in "
       "Smp.String8 p = nullptr) }",
       false},
      // Fields of a value.
      {"struct H5 { field Smp.Int32 f = nullptr }", false},
      {"struct H6 { field Name f = nullptr }", false},
      {"struct H7 { field Point f = nullptr }", false},
      {"struct H8 { field Pair f = nullptr }", false},
      {"struct H9 { field Target f = nullptr }", false},
      {"struct I1 { field Pair f = {nullptr, 1} }", false},
      {"struct I2 { field Point f = {.x = nullptr} }", false},
      {"struct I3 { field Holder f = {nullptr} }", false},
      // A value that has to be a number.
      {"string I4[nullptr]", false},
      {"array I6 = Smp.Int32[nullptr]", false},
      // An attribute writes no C++.
      {"attribute Smp.String8 J1 = nullptr", false},
      {"@Text(nullptr) struct J3 { }", false},
  };

  std::string text = R"(catalogue c namespace n {
    public string Name[8]
    public enum Mode { Off = 0 }
    public array Pair = Smp.Int32[2]
    public array Texts = Smp.String8[2]
    public struct Point { field Smp.Int32 x }
    public struct Holder { field Target target }
    public struct Nullable { field Smp.String8 text field Ref ref }
    public class Thing { }
    public interface Target { }
    public model Unit { }
    public using Ref = Smp.Int32*
    public native Handle
    public attribute Smp.String8 Text = ""
  )";
  const auto firstLine = std::ranges::count(text, '\n');
  for (const auto& [line, valid] : cases) text += std::string(line) + "\n";
  text += "}\n";

  auto ws = workspaceWithBuiltins();
  const auto result = validate(ws, text);
  std::vector<bool> refused(std::size(cases));
  for (const auto& diagnostic : result.document->diagnostics)
    if (diagnostic.message == xsmp::nullptrNeedsAPointer
        || diagnostic.message.starts_with("A non-const reference parameter")) {
      const auto line =
          std::count(text.begin(), text.begin() + diagnostic.range.begin, '\n')
          - firstLine;
      ASSERT_GE(line, 0) << text.substr(diagnostic.range.begin, 40);
      refused[static_cast<std::size_t>(line)] = true;
    }
  for (std::size_t i = 0; i < std::size(cases); ++i)
    EXPECT_EQ(refused[i], !cases[i].second) << cases[i].first;
}

// The solver cannot read a value from `nullptr`, so nothing else complains.
TEST(XsmpcatValidator, ValidNullptrHasNoOtherDiagnostic) {
  auto ws = workspaceWithBuiltins();
  const auto result = validate(ws, R"(catalogue c namespace n {
    /** @uuid 11111111-1111-1111-1111-111111111110 */
    public using Ref = Smp.Int32*
    /** @uuid 11111111-1111-1111-1111-111111111111 */
    public struct S { field Ref a = nullptr }
    /** @uuid 11111111-1111-1111-1111-111111111112 */
    public interface I {
      def void f(out Smp.Int32 a = nullptr, in Ref b = nullptr)
    }
  }
)");
  EXPECT_EQ(errorsOf(result), std::vector<std::string>{});
}

// Only the constant is reported: what names it reads no value from it.
// A pointer takes `nullptr` only, and a reference that can change its argument
// has nothing to bind to.
TEST(XsmpcatValidator, AParameterDefaultFollowsHowItIsPassed) {
  auto ws = workspaceWithBuiltins();
  const auto result = validate(ws, R"(catalogue c
namespace n {
  /** @uuid 11111111-0000-4000-8000-000000000001 */
  public interface I {
    def void pointer(@Attributes.ByPointer in Smp.Int32 p = 1)
    def void out(out Smp.Int32 p = 1)
    def void reference(@Attributes.ByReference inout Smp.Int32 p = 1)
    def void optional(@Attributes.ByPointer in Smp.Int32 p = (nullptr))
    def void constant(@Attributes.ByReference @Attributes.Const in Smp.Int32 p = 1)
  }
}
)");
  EXPECT_EQ(errorsOf(result),
            (std::vector<std::string>{
                "A pointer parameter default shall be nullptr.",
                "A pointer parameter default shall be nullptr.",
                "A non-const reference parameter cannot bind to a default "
                "value expression."}));
}

// A type registers its bounds as Int64.
TEST(XsmpcatValidator, AUInt64UpperBoundPastInt64IsAWarning) {
  const auto warnings = [](std::string_view declaration) {
    auto ws = workspaceWithBuiltins();
    const auto result = validate(
        ws,
        "catalogue c\nnamespace n {\n/** @uuid "
        "11111111-0000-4000-8000-000000000001 */\npublic integer U extends "
            + std::string(declaration) + "\n}\n");
    return std::ranges::count_if(
        result.document->diagnostics, [](const auto& diagnostic) {
          return diagnostic.message.starts_with("The UInt64 upper bound");
        });
  };
  EXPECT_EQ(warnings("Smp.UInt64"), 1);
  EXPECT_EQ(warnings("Smp.UInt64 in 0 ... 9223372036854775808"), 1);
  EXPECT_EQ(warnings("Smp.UInt64 in 0 ... 9223372036854775807"), 0);
  EXPECT_EQ(warnings("Smp.Int64"), 0);
}

TEST(XsmpcatValidator, AConstantCannotBeNullptr) {
  auto ws = workspaceWithBuiltins();
  const auto result = validate(ws, R"(catalogue c namespace n {
    /** @uuid 11111111-1111-1111-1111-111111111111 */
    public struct S {
      constant Smp.String8 a = nullptr
      constant Smp.Int32 b = (nullptr)
      constant Smp.String8 c = a
      field Smp.String8 d = a
    }
    /** @uuid 11111111-1111-1111-1111-111111111112 */
    public interface I { def void f(in Smp.String8 p = S.a) }
  }
)");
  EXPECT_EQ(errorsOf(result),
            (std::vector<std::string>{xsmp::nullptrNeedsAPointer,
                                      xsmp::nullptrNeedsAPointer}));
}

// Every literal holds its bytes, with nothing said of it.
TEST(XsmpcatValidator, EveryEscapeHoldsItsValue) {
  std::string text = R"(catalogue c namespace n {
    /** @uuid 11111111-1111-1111-1111-111111111111 */
    public struct S {
)";
  const auto& cases = xsmp::escapes::cases();
  for (const auto& c : cases)
    text += std::string("constant ")
            + (c.isChar() ? "Smp.Char8" : "Smp.String8")
            + " k = " + std::string(c.source) + "\n";
  text += "} }\n";
  auto ws = workspaceWithBuiltins();
  const auto result = validate(ws, text);
  // Each constant has the one name: only that may be said.
  for (const auto& error : errorsOf(result))
    EXPECT_NE(error.find("Duplicate"), std::string::npos) << error;
  const auto constants =
      xsmp::allContent<xsmp::Constant>(*result.document->parseResult.value);
  ASSERT_EQ(constants.size(), cases.size());
  for (std::size_t i = 0; i < cases.size(); ++i) {
    const auto value = xsmp::getValue(constants[i]->value);
    ASSERT_TRUE(value.has_value()) << cases[i].source;
    EXPECT_EQ(value->text, cases[i].value) << cases[i].source;
  }
}

TEST(XsmpcatValidator, EveryRefusedLiteralSaysWhy) {
  for (const auto& [source, message] : xsmp::escapes::refused) {
    SCOPED_TRACE(source);
    auto ws = workspaceWithBuiltins();
    const auto result = validate(
        ws, std::string(R"(catalogue c namespace n {
          /** @uuid 11111111-1111-1111-1111-111111111111 */
          public struct S { constant )")
                + (source.front() == '\'' ? "Smp.Char8" : "Smp.String8")
                + " k = " + std::string(source) + "\n} }\n");
    // One fault, one message.
    EXPECT_EQ(errorsOf(result), std::vector<std::string>{std::string(message)});
  }
}

// Only the literal that holds an unknown escape says so, at that escape.
TEST(XsmpcatValidator, AnUnknownEscapeIsAWarningAtTheEscape) {
  for (const auto& [source, message] : xsmp::escapes::unknownEscapes) {
    SCOPED_TRACE(source);
    auto ws = workspaceWithBuiltins();
    const std::string head = R"(catalogue c namespace n {
      /** @uuid 11111111-1111-1111-1111-111111111111 */
      public struct S { constant )";
    const auto text = head
                      + (source.front() == '\'' ? "Smp.Char8" : "Smp.String8")
                      + " k = " + std::string(source) + " /* \\d */ }\n}\n";
    const auto result = validate(ws, text);
    EXPECT_EQ(errorsOf(result), std::vector<std::string>{});
    std::vector<std::string> warnings;
    for (const auto& diagnostic : result.document->diagnostics)
      if (diagnostic.severity == cursiv::DiagnosticSeverity::Warning
          && diagnostic.message.starts_with("Unknown escape")) {
        warnings.push_back(diagnostic.message);
        EXPECT_EQ(text.substr(diagnostic.range.begin,
                              diagnostic.range.end - diagnostic.range.begin),
                  message.substr(message.find('\\'),
                                 message.size() - message.find('\\') - 1));
      }
    EXPECT_EQ(warnings, std::vector<std::string>{std::string(message)});
  }
}

TEST(XsmpcatValidator, TheEscapesOfCppHaveNoWarning) {
  auto ws = workspaceWithBuiltins();
  const auto result = validate(ws, R"(catalogue c namespace n {
    /** @uuid 11111111-1111-1111-1111-111111111111 */
    public struct S {
      constant Smp.String8 a = "\'\"\?\\\a\b\f\n\r\t\v\0\177\x7f\u0041\U00000041"
      constant Smp.Char8 b = '\''
    }
  }
)");
  for (const auto& diagnostic : result.document->diagnostics)
    EXPECT_EQ(diagnostic.message.find("Unknown escape"), std::string::npos)
        << diagnostic.message;
}

// What did not decode has no value: what uses it, or its type, says nothing
// more.
TEST(XsmpcatValidator, ALiteralThatDoesNotDecodeIsReportedOnce) {
  auto ws = workspaceWithBuiltins();
  const auto result = validate(ws, R"(catalogue c namespace n {
    /** @uuid 11111111-1111-1111-1111-111111111111 */
    public string Short[1]
    /** @uuid 11111111-1111-1111-1111-111111111112 */
    public struct S {
      constant Smp.Char8 a = '\x'
      constant Smp.Char8 b = a
      field Smp.Char8 c = '\x'
      constant Smp.String8 d = "ok" "\x"
      field Short e = "abc\x"
      constant Smp.Duration f = "\x"
      constant Smp.DateTime g = "\x"
    }
  }
)");
  EXPECT_EQ(errorsOf(result), std::vector<std::string>(
                                  6, std::string(xsmp::escapes::noHexDigit)));
}

// A string type counts the bytes of the value, not its characters: exactly that
// many fit, and one less does not.
TEST(XsmpcatValidator, TheLengthOfAStringTypeCountsBytes) {
  std::string text = "catalogue c namespace n {\n";
  std::size_t tooLong = 0, types = 0;
  const auto withLength = [&](const xsmp::escapes::Case& c,
                              std::size_t length) {
    const auto name = "T" + std::to_string(types++);
    text += "public string " + name + "[" + std::to_string(length)
            + "]\npublic struct S" + name + " { field " + name
            + " f = " + std::string(c.source) + " }\n";
  };
  for (const auto& c : xsmp::escapes::cases()) {
    if (c.isChar()) continue;
    withLength(c, c.value.size());
    if (!c.value.empty()) {
      withLength(c, c.value.size() - 1);
      ++tooLong;
    }
  }
  text += "}\n";
  auto ws = workspaceWithBuiltins();
  const auto result = validate(ws, text);
  EXPECT_EQ(std::ranges::count(
                errorsOf(result),
                "The string length exceeds the allowed length for its type."),
            static_cast<std::ptrdiff_t>(tooLong));
}

TEST(XsmpcatValidator, EnumerationWithoutLiteral) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(ws, "catalogue c\nnamespace n { enum E {} }\n");
  cursiv::test::expectError(result,
                            cursiv::test::contains("at least one literal"));
}

TEST(XsmpcatValidator, EnumerationDuplicateValue) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result =
      validate(ws, "catalogue c\nnamespace n { enum E { A = 0, B = 0 } }\n");
  cursiv::test::expectError(
      result,
      cursiv::test::contains(
          "Enumeration Literal Values shall be unique within an Enumeration."));
}

// An enumeration literal's value must itself convert to Int32; a failed
// conversion is reported.
TEST(XsmpcatValidator, EnumerationLiteralValueOverflowIsInvalid) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result =
      validate(ws, "catalogue c\nnamespace n { enum E { A = 5000000000 } }\n");
  cursiv::test::expectError(result, cursiv::test::contains("is out of range"));
}

TEST(XsmpcatValidator, IntegerMinGreaterThanMax) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result =
      validate(ws, "catalogue c\nnamespace n { integer I in 5 ... 3 }\n");
  cursiv::test::expectError(
      result, cursiv::test::contains("Minimum shall be less or equal"));
}

TEST(XsmpcatValidator, FloatExclusiveRangeEqualBounds) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result =
      validate(ws, "catalogue c\nnamespace n { float F in 1.0 <.< 1.0 }\n");
  cursiv::test::expectError(
      result, cursiv::test::contains("Minimum shall be less than Maximum"));
}

// The `in <min> ... <max>` bounds are checked against the storage primitive's
// own representable range, not just against each other.
TEST(XsmpcatValidator, IntegerBoundOutOfRangeForStoragePrimitiveIsInvalid) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result =
      validate(ws,
               "catalogue c\n"
               "namespace Smp { public primitive Int8 }\n"
               "namespace n { integer I extends Smp.Int8 in -300 ... 300 }\n");
  cursiv::test::expectError(result, cursiv::test::contains("is out of range"));
}

// A string length that is not an integer is reported.
TEST(XsmpcatValidator, StringLengthNotConvertibleToIntegerIsInvalid) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result =
      validate(ws, "catalogue c\nnamespace n { string S[\"abc\"] }\n");
  cursiv::test::expectError(result,
                            cursiv::test::contains("cannot be converted"));
}

// An array size that is not an integer is reported.
TEST(XsmpcatValidator, ArraySizeNotConvertibleToIntegerIsInvalid) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(ws,
                         "catalogue c\n"
                         "namespace n {\n"
                         "  primitive Int32\n"
                         "  array A = Int32[\"xyz\"]\n"
                         "}\n");
  cursiv::test::expectError(result,
                            cursiv::test::contains("cannot be converted"));
}

TEST(XsmpcatValidator, CyclicClassBase) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result =
      validate(ws, "catalogue c\nnamespace n { class A extends A {} }\n");
  cursiv::test::expectError(result,
                            cursiv::test::contains("Cyclic dependency"));
}

// The Class checks also run for `exception` declarations.
TEST(XsmpcatValidator, CyclicExceptionBase) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result =
      validate(ws, "catalogue c\nnamespace n { exception E extends E {} }\n");
  cursiv::test::expectError(result,
                            cursiv::test::contains("Cyclic dependency"));
}

// The Component checks run for both Model and Service.
TEST(XsmpcatValidator, CyclicModelBase) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result =
      validate(ws, "catalogue c\nnamespace n { model M extends M {} }\n");
  cursiv::test::expectError(result,
                            cursiv::test::contains("Cyclic dependency"));
}

TEST(XsmpcatValidator, CyclicServiceBase) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result =
      validate(ws, "catalogue c\nnamespace n { service S extends S {} }\n");
  cursiv::test::expectError(result,
                            cursiv::test::contains("Cyclic dependency"));
}

// The base of a Model, Service or Exception is narrowed per rule: a Model can
// only extend a Model, not a sibling Service.
TEST(XsmpcatValidator, ModelCannotExtendService) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result =
      validate(ws,
               "catalogue c\n"
               "namespace n { service Svc {} model M extends Svc {} }\n");
  cursiv::test::expectError(
      result, cursiv::test::contains("cannot be used here: expected Model"));
}

TEST(XsmpcatValidator, ServiceCannotExtendModel) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result =
      validate(ws,
               "catalogue c\n"
               "namespace n { model Mdl {} service S extends Mdl {} }\n");
  cursiv::test::expectError(
      result, cursiv::test::contains("cannot be used here: expected Service"));
}

// An Exception's base must itself be an Exception — a plain (non-exception)
// Class is not enough, even though Exception IS-A Class structurally.
TEST(XsmpcatValidator, ExceptionCannotExtendPlainClass) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result =
      validate(ws,
               "catalogue c\n"
               "namespace n { class C {} exception E extends C {} }\n");
  cursiv::test::expectError(
      result,
      cursiv::test::contains("cannot be used here: expected Exception"));
}

// The reverse direction is fine: a plain Class may extend an Exception, since
// Exception IS-A Class.
TEST(XsmpcatValidator, ClassExtendingExceptionHasNoError) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(ws,
                         "catalogue c\n"
                         "namespace n\n"
                         "{\n"
                         "  /** @uuid 11111111-1111-1111-1111-111111111111 */\n"
                         "  exception E {}\n"
                         "  /** @uuid 22222222-2222-2222-2222-222222222222 */\n"
                         "  class C extends E {}\n"
                         "}\n");
  cursiv::test::expectNoIssues(result,
                               {.severity = cursiv::DiagnosticSeverity::Error});
}

// A plain Class must still be Class-family — not, say, an Interface.
TEST(XsmpcatValidator, ClassCannotExtendInterface) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result =
      validate(ws,
               "catalogue c\n"
               "namespace n { interface I {} class C extends I {} }\n");
  cursiv::test::expectError(
      result, cursiv::test::contains("cannot be used here: expected Class"));
}

// Duplicate-member detection must also walk the inheritance chain: a derived
// Class redeclaring a base Field's name is a collision, not shadowing.
TEST(XsmpcatValidator, FieldShadowingInheritedFieldIsDuplicate) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(ws,
                         "catalogue c\n"
                         "namespace n\n"
                         "{\n"
                         "  primitive Bool\n"
                         "  class Base { field Bool x }\n"
                         "  class Derived extends Base { field Bool x }\n"
                         "}\n");
  cursiv::test::expectError(result, cursiv::test::contains("Duplicated name"));
  const auto duplicate = std::ranges::find_if(
      result.document->diagnostics,
      [](const auto& d) { return d.message.starts_with("Duplicated name"); });
  ASSERT_NE(duplicate, result.document->diagnostics.end());
  ASSERT_EQ(duplicate->relatedInformation.size(), 1u);
  EXPECT_EQ(duplicate->relatedInformation[0].message, "n.Base.x");
}

// ...but a Property/Constant of the same name as one inherited from a base IS
// allowed: only a collision within the same container is flagged for these two
// kinds.
TEST(XsmpcatValidator, PropertyShadowingInheritedPropertyIsAllowed) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(
      ws,
      "catalogue c\n"
      "namespace n\n"
      "{\n"
      "  primitive Bool\n"
      "  class Base { field Bool f public property Bool p -> f }\n"
      "  class Derived extends Base { field Bool f2 public property Bool "
      "p -> f2 }\n"
      "}\n");
  cursiv::test::expectNoIssues(
      result, {.messageMatches = cursiv::test::contains("Duplicated name")});
}

// An Operation colliding by name with a non-Operation member is a duplicate.
TEST(XsmpcatValidator, OperationCollidingWithFieldIsDuplicate) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(ws,
                         "catalogue c\n"
                         "namespace n\n"
                         "{\n"
                         "  primitive Bool\n"
                         "  class C { field Bool x def void x() }\n"
                         "}\n");
  cursiv::test::expectError(result, cursiv::test::contains("Duplicated name"));
}

// Two Operations with DIFFERENT signatures in the same class are a legitimate
// overload — not flagged.
TEST(XsmpcatValidator, OverloadedOperationsAreNotDuplicates) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(ws,
                         "catalogue c\n"
                         "namespace n\n"
                         "{\n"
                         "  primitive Bool\n"
                         "  class C { def void f() def void f(in Bool a) }\n"
                         "}\n");
  cursiv::test::expectNoIssues(
      result, {.messageMatches = cursiv::test::contains("Duplicated name")});
}

// Two Operations with the SAME signature declared twice in the SAME class are
// an exact redeclaration, not an overload — flagged.
TEST(XsmpcatValidator, SameSignatureOperationTwiceIsDuplicate) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result =
      validate(ws,
               "catalogue c\n"
               "namespace n { class C { def void f() def void f() } }\n");
  cursiv::test::expectError(result, cursiv::test::contains("Duplicated name"));
}

// ...but overriding a BASE class's operation with the same signature in a
// derived class is allowed (not an exact redeclaration in the same container).
TEST(XsmpcatValidator, OverridingBaseOperationSameSignatureIsAllowed) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(ws,
                         "catalogue c\n"
                         "namespace n\n"
                         "{\n"
                         "  class Base { def void f() }\n"
                         "  class Derived extends Base { def void f() }\n"
                         "}\n");
  cursiv::test::expectNoIssues(
      result, {.messageMatches = cursiv::test::contains("Duplicated name")});
}

// The Structure check does not run for a Class: it would duplicate the Class
// diagnostics and apply the narrower modifier rules of a Structure field. A
// valid `public field` of a Class gets no "is not allowed here.", and a
// duplicate name is reported once.
TEST(XsmpcatValidator, ClassDoesNotDoubleRunStructureCheck) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(ws,
                         "catalogue c\n"
                         "namespace n\n"
                         "{\n"
                         "  primitive P\n"
                         "  /** @uuid 11111111-1111-1111-1111-111111111111 */\n"
                         "  class A\n"
                         "  {\n"
                         "    public field P x\n"
                         "    public field P x\n"
                         "  }\n"
                         "}\n");
  std::size_t duplicatedIdentifierCount = 0;
  for (const auto& diagnostic : result.diagnostics) {
    EXPECT_EQ(diagnostic.message.find("is not allowed here"), std::string::npos)
        << diagnostic.message;
    if (diagnostic.message.find("Duplicated name") != std::string::npos) {
      ++duplicatedIdentifierCount;
    }
  }
  EXPECT_EQ(duplicatedIdentifierCount, 1U);
}

TEST(XsmpcatValidator, InvalidModifier) {
  auto ws = xsmp::xsmpcatWorkspace();
  // `public` parses on a field (FieldModifiers), but a struct field only
  // accepts input/output/transient — so it is reported as an invalid modifier.
  auto result = validate(ws,
                         "catalogue c\n"
                         "namespace n\n"
                         "{\n"
                         "  primitive P\n"
                         "  struct S { public field P x }\n"
                         "}\n");
  cursiv::test::expectError(result,
                            cursiv::test::contains("is not allowed here"));
}

TEST(XsmpcatValidator, DuplicatedMember) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(ws,
                         "catalogue c\n"
                         "namespace n\n"
                         "{\n"
                         "  primitive P\n"
                         "  struct S { field P a field P a }\n"
                         "}\n");
  cursiv::test::expectError(result, cursiv::test::contains("Duplicated name"));
}

TEST(XsmpcatValidator, FieldTypeMustBeLanguageType) {
  auto ws = xsmp::xsmpcatWorkspace();
  // `E` is an EventType (a Type but not a LanguageType) → invalid field type.
  auto result = validate(ws,
                         "catalogue c\n"
                         "namespace n\n"
                         "{\n"
                         "  event E\n"
                         "  struct S { field E x }\n"
                         "}\n");
  cursiv::test::expectError(
      result, cursiv::test::contains(
                  "(EventType) cannot be used here: expected LanguageType."));
}

// A Field of a Model or Service shall have a ValueType type.

TEST(XsmpcatValidator, ModelFieldMustBeValueType) {
  auto ws = xsmp::xsmpcatWorkspace();
  // `I` is an Interface (a LanguageType but not a ValueType).
  auto result = validate(ws,
                         "catalogue c\n"
                         "namespace n\n"
                         "{\n"
                         "  interface I {}\n"
                         "  /** @uuid 11111111-1111-1111-1111-111111111111 */\n"
                         "  model M { field I x }\n"
                         "}\n");
  cursiv::test::expectError(
      result, cursiv::test::contains(
                  "A Field of a Model or Service shall have a ValueType type"));
}

TEST(XsmpcatValidator, NonComponentFieldNonValueTypeWarnsInstead) {
  auto ws = xsmp::xsmpcatWorkspace();
  // Same scenario, but the field is on a plain Structure: a warning, not an
  // error, since a ValueType is only required inside a Component.
  auto result = validate(ws,
                         "catalogue c\n"
                         "namespace n\n"
                         "{\n"
                         "  interface I {}\n"
                         "  struct S { field I x }\n"
                         "}\n");
  cursiv::test::expectWarning(
      result, cursiv::test::contains("Field type is not a ValueType"));
}

TEST(XsmpcatValidator, ModelFieldRejectsValueTypeNestingNonValueTypeField) {
  auto ws = xsmp::xsmpcatWorkspace();
  // `Inner` (a Structure, itself a ValueType) nests a non-ValueType field —
  // rejected even though the OUTER field's own type (`Inner`) is a ValueType.
  auto result = validate(ws,
                         "catalogue c\n"
                         "namespace n\n"
                         "{\n"
                         "  interface I {}\n"
                         "  struct Inner { field I bad }\n"
                         "  /** @uuid 11111111-1111-1111-1111-111111111111 */\n"
                         "  model M { field Inner nested }\n"
                         "}\n");
  cursiv::test::expectError(
      result,
      cursiv::test::contains(
          "A Field of a Model or Service shall not use a type containing "
          "non-ValueType fields"));
}

TEST(XsmpcatValidator, ModelFieldWithValueTypeHasNoIssue) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(ws,
                         "catalogue c\n"
                         "namespace n\n"
                         "{\n"
                         "  primitive Bool\n"
                         "  /** @uuid 11111111-1111-1111-1111-111111111111 */\n"
                         "  model M { field Bool x }\n"
                         "}\n");
  cursiv::test::expectNoIssues(
      result, {.messageMatches = cursiv::test::contains("ValueType")});
}

TEST(XsmpcatValidator, IntegerFieldValueExceedsMaximum) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(ws,
                         "catalogue c\n"
                         "namespace n\n"
                         "{\n"
                         "  integer Small in 0 ... 10\n"
                         "  struct S { field Small x = 50 }\n"
                         "}\n");
  cursiv::test::expectError(
      result, cursiv::test::contains(
                  "Integral value shall be less than or equal to 10."));
}

TEST(XsmpcatValidator, IntegerConstantValueBelowMinimum) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(ws,
                         "catalogue c\n"
                         "namespace n\n"
                         "{\n"
                         "  integer Small in 0 ... 10\n"
                         "  struct S { constant Small c = -5 }\n"
                         "}\n");
  cursiv::test::expectError(
      result, cursiv::test::contains(
                  "Integral value shall be greater than or equal to 0."));
}

TEST(XsmpcatValidator, StringConstantExceedsLength) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(ws,
                         "catalogue c\n"
                         "namespace n\n"
                         "{\n"
                         "  string Str3 [3]\n"
                         "  struct S { constant Str3 c = \"toolong\" }\n"
                         "}\n");
  cursiv::test::expectError(
      result,
      cursiv::test::contains(
          "The string length exceeds the allowed length for its type."));
}

// Each bound of a float range has its own exclusion flag: the maximum of
// `0.0 <.. 1.0` is inclusive, the maximum of `0.0 ..< 1.0` is exclusive.
TEST(XsmpcatValidator, FloatMaximumHonoursItsOwnExclusionFlag) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(ws,
                         "catalogue c\n"
                         "namespace n\n"
                         "{\n"
                         "  float Open in 0.0 <.. 1.0\n"
                         "  float Closed in 0.0 ..< 1.0\n"
                         "  struct S {\n"
                         "    constant Open a = 1.0\n"
                         "    constant Closed b = 1.0\n"
                         "  }\n"
                         "}\n");
  cursiv::test::expectError(
      result, cursiv::test::contains("Float value shall be less than 1."));
  EXPECT_EQ(std::ranges::count_if(result.document->diagnostics,
                                  [](const auto& diagnostic) {
                                    return diagnostic.message.starts_with(
                                        "Float value shall be");
                                  }),
            1);
}

TEST(XsmpcatValidator, Unsigned64BoundsUseUnsignedOrdering) {
  auto ws = xsmp::xsmpcatWorkspace();
  xsmp::loadXsmpcatBuiltins(ws.shared());
  auto result = validate(ws, R"(catalogue c namespace n {
    integer Wide extends Smp.UInt64 in 0 ... 18446744073709551614UL
    struct K { constant Wide good = 9223372036854775808UL constant Wide bad = 18446744073709551615UL }
  })");
  cursiv::test::expectNoIssues(
      result,
      {.messageMatches = cursiv::test::contains("Minimum shall be less")});
  cursiv::test::expectError(
      result, cursiv::test::contains("Integral value shall be less than or "
                                     "equal to 18446744073709551614."));
}

TEST(XsmpcatValidator,
     ExportedLiteralsAndConstantsParticipateInNameCollisions) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(ws, R"(catalogue c namespace n {
    enum E { value = 0 }
    namespace E { primitive value }
    struct S { constant Smp.Int32 value = 0 }
    namespace S { namespace value {} }
  })");
  std::size_t typeCollisions = 0, namespaceCollisions = 0;
  for (const auto& diagnostic : result.document->diagnostics) {
    if (diagnostic.message == "Duplicated Type name.") ++typeCollisions;
    if (diagnostic.message == "Duplicated name.") ++namespaceCollisions;
  }
  // E/S themselves collide, and n.E.value / n.S.value additionally collide
  // with the exported literal / constant.
  EXPECT_EQ(typeCollisions, 3u);
  EXPECT_EQ(namespaceCollisions, 3u);
}

TEST(XsmpcatValidator, MissingTypeUuid) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(ws, "catalogue c\nnamespace n { primitive P }\n");
  cursiv::test::expectError(result,
                            cursiv::test::contains("Missing Type UUID"));
}

TEST(XsmpcatValidator, InvalidTypeUuid) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(ws,
                         "catalogue c\n"
                         "namespace n\n"
                         "{\n"
                         "  /** @uuid not-a-uuid */\n"
                         "  primitive P\n"
                         "}\n");
  cursiv::test::expectError(result,
                            cursiv::test::contains("The UUID is invalid"));
}

// Doc comments only know ASCII blanks: a no-break space does not separate a tag
// from its value, a tab does.
TEST(XsmpcatValidator, DocCommentBlanksAreAscii) {
  const std::string uuid = "11111111-1111-1111-1111-111111111111";
  auto ws = xsmp::xsmpcatWorkspace();
  cursiv::test::expectError(
      validate(ws, "catalogue c\nnamespace n {\n  /** @uuid\xC2\xA0" + uuid
                       + " */\n  primitive P\n}\n"),
      cursiv::test::contains("The UUID is invalid"));
  cursiv::test::expectNoIssues(
      validate(ws, "catalogue c\nnamespace n {\n  /** @uuid\t" + uuid
                       + " */\n  primitive P\n}\n"),
      {.messageMatches = cursiv::test::contains("UUID")});
}

// "The UUID is invalid." anchors on the `@uuid` tag's own text inside the doc
// comment, not the whole `primitive P` declaration.
TEST(XsmpcatValidator, InvalidTypeUuidAnchorsOnTheUuidTag) {
  auto ws = xsmp::xsmpcatWorkspace();
  const std::string text =
      "catalogue c\n"
      "namespace n\n"
      "{\n"
      "  /** @uuid not-a-uuid */\n"
      "  primitive P\n"
      "}\n";
  auto result = validate(ws, text);
  const auto primitiveBegin = text.find("primitive P");
  ASSERT_NE(primitiveBegin, std::string::npos);
  auto found = false;
  for (const auto& diagnostic : result.diagnostics) {
    if (diagnostic.message.find("The UUID is invalid") != std::string::npos) {
      EXPECT_LT(diagnostic.range.begin, primitiveBegin)
          << "expected the diagnostic to anchor inside the doc comment, "
             "before the `primitive P` declaration";
      EXPECT_LT(diagnostic.range.end, primitiveBegin);
      found = true;
    }
  }
  EXPECT_TRUE(found);
}

TEST(XsmpcatValidator, NegationKeepsTheExactInt64Range) {
  auto ws = xsmp::xsmpcatWorkspace();
  xsmp::loadXsmpcatBuiltins(ws.shared<xsmp::XsmpSharedServices>());
  cursiv::test::expectNoIssues(
      validate(
          ws,
          "catalogue c\nnamespace n\n{\n  struct K\n  {\n"
          "    constant Smp.Int64 lowest = -9223372036854775808\n  }\n}\n"),
      {.messageMatches = cursiv::test::contains("cannot be converted")});
  cursiv::test::expectError(
      validate(ws,
               "catalogue c\nnamespace n\n{\n  struct K\n  {\n"
               "    constant Smp.Int64 below = -9223372036854775809\n  }\n}\n"),
      cursiv::test::contains("Conversion overflow for type Int64."));
}

TEST(XsmpcatValidator, DuplicatedTypeUuid) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(ws,
                         "catalogue c\n"
                         "namespace n\n"
                         "{\n"
                         "  /** @uuid 11111111-1111-1111-1111-111111111111 */\n"
                         "  primitive P\n"
                         "  /** @uuid 11111111-1111-1111-1111-111111111111 */\n"
                         "  primitive Q\n"
                         "}\n");
  cursiv::test::expectError(result, cursiv::test::contains("Duplicated UUID"));
}

TEST(XsmpcatValidator, NativeTypeMissingTypeTag) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(ws,
                         "catalogue c\n"
                         "namespace n\n"
                         "{\n"
                         "  /** @uuid 11111111-1111-1111-1111-111111111111 */\n"
                         "  native NT\n"
                         "}\n");
  cursiv::test::expectError(result, cursiv::test::contains("@type"));
}

TEST(XsmpcatValidator, RecursiveFieldType) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(ws,
                         "catalogue c\n"
                         "namespace n\n"
                         "{\n"
                         "  /** @uuid 11111111-1111-1111-1111-111111111111 */\n"
                         "  struct S { field S x }\n"
                         "}\n");
  cursiv::test::expectError(result,
                            cursiv::test::contains("Recursive Field Type"));
}

TEST(XsmpcatValidator, MultiplicityLowerGreaterThanUpper) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(ws,
                         "catalogue c\n"
                         "namespace n\n"
                         "{\n"
                         "  /** @uuid 11111111-1111-1111-1111-111111111111 */\n"
                         "  interface I {}\n"
                         "  /** @uuid 22222222-2222-2222-2222-222222222222 */\n"
                         "  model M { reference I[5 ... 3] refs }\n"
                         "}\n");
  cursiv::test::expectError(
      result, cursiv::test::contains("Lower bound shall be less or equal"));
}

TEST(XsmpcatValidator, DefaultComponentNotSubtypeOfContainerTypeIsError) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(ws,
                         "catalogue c\n"
                         "namespace n\n"
                         "{\n"
                         "  /** @uuid 11111111-1111-1111-1111-111111111111 */\n"
                         "  interface I {}\n"
                         "  /** @uuid 22222222-2222-2222-2222-222222222222 */\n"
                         "  model M1 { container I[*] items = M2 }\n"
                         "  /** @uuid 33333333-3333-3333-3333-333333333333 */\n"
                         "  model M2 {}\n"
                         "}\n");
  cursiv::test::expectError(result,
                            cursiv::test::contains("shall be a sub type of"));
}

TEST(XsmpcatValidator, DefaultComponentSubtypeOfContainerTypeHasNoError) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(ws,
                         "catalogue c\n"
                         "namespace n\n"
                         "{\n"
                         "  /** @uuid 11111111-1111-1111-1111-111111111111 */\n"
                         "  interface I {}\n"
                         "  /** @uuid 22222222-2222-2222-2222-222222222222 */\n"
                         "  model M1 { container I[*] items = M2 }\n"
                         "  /** @uuid 33333333-3333-3333-3333-333333333333 */\n"
                         "  model M2 implements I {}\n"
                         "}\n");
  for (const auto& diagnostic : result.diagnostics) {
    EXPECT_EQ(diagnostic.message.find("shall be a sub type of"),
              std::string::npos);
  }
}

TEST(XsmpcatValidator, ReferenceToNonInterfaceInEcssSmp2020IsError) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(ws,
                         "catalogue c\n"
                         "namespace n\n"
                         "{\n"
                         "  /** @uuid 11111111-1111-1111-1111-111111111111 */\n"
                         "  model M {}\n"
                         "  /** @uuid 22222222-2222-2222-2222-222222222222 */\n"
                         "  model M2 { reference M target }\n"
                         "}\n");
  cursiv::test::expectError(
      result, cursiv::test::contains(
                  "A Reference in ECSS_SMP_2020 shall target an Interface."));
}

TEST(XsmpcatValidator, ReferenceToInterfaceInEcssSmp2020HasNoError) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(ws,
                         "catalogue c\n"
                         "namespace n\n"
                         "{\n"
                         "  /** @uuid 11111111-1111-1111-1111-111111111111 */\n"
                         "  interface I {}\n"
                         "  /** @uuid 22222222-2222-2222-2222-222222222222 */\n"
                         "  model M2 { reference I target }\n"
                         "}\n");
  for (const auto& diagnostic : result.diagnostics) {
    EXPECT_EQ(diagnostic.message.find("shall target an Interface"),
              std::string::npos);
  }
}

TEST(XsmpcatValidator, SimpleArrayOfNonSimpleTypeIsError) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(ws,
                         "catalogue c\n"
                         "namespace Attributes {\n"
                         "  public primitive Flag\n"
                         "  public attribute Flag SimpleArray = true\n"
                         "}\n"
                         "namespace n {\n"
                         "  /** @uuid 11111111-1111-1111-1111-111111111111 */\n"
                         "  primitive P\n"
                         "  /** @uuid 22222222-2222-2222-2222-222222222222 */\n"
                         "  struct S { field P x }\n"
                         "  /** @uuid 33333333-3333-3333-3333-333333333333 */\n"
                         "  @Attributes.SimpleArray\n"
                         "  array A = S[4]\n"
                         "}\n");
  cursiv::test::expectError(result, cursiv::test::contains("SimpleArray"));
}

TEST(XsmpcatValidator, SimpleArrayOfSimpleTypeHasNoError) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(ws,
                         "catalogue c\n"
                         "namespace Attributes {\n"
                         "  public primitive Flag\n"
                         "  public attribute Flag SimpleArray = true\n"
                         "}\n"
                         "namespace n {\n"
                         "  /** @uuid 11111111-1111-1111-1111-111111111111 */\n"
                         "  primitive P\n"
                         "  /** @uuid 22222222-2222-2222-2222-222222222222 */\n"
                         "  @Attributes.SimpleArray\n"
                         "  array A = P[4]\n"
                         "}\n");
  for (const auto& diagnostic : result.diagnostics) {
    EXPECT_EQ(diagnostic.message.find("SimpleArray"), std::string::npos);
  }
}

TEST(XsmpcatValidator, EntryPointInputMustBeInputField) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(ws,
                         "catalogue c\n"
                         "namespace n\n"
                         "{\n"
                         "  /** @uuid 11111111-1111-1111-1111-111111111111 */\n"
                         "  primitive P\n"
                         "  /** @uuid 22222222-2222-2222-2222-222222222222 */\n"
                         "  model M\n"
                         "  {\n"
                         "    field P notInput\n"
                         "    entrypoint ep { in notInput }\n"
                         "  }\n"
                         "}\n");
  cursiv::test::expectError(result,
                            cursiv::test::contains("Field is not an Input"));
}

// An EntryPoint's `in`/`out` may resolve a private Field INHERITED from a base
// Model (the inherited-member scope does not filter by visibility), but the
// visibility check must reject it.
TEST(XsmpcatValidator, EntryPointCannotReferencePrivateFieldFromBaseModel) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result =
      validate(ws,
               "catalogue c\n"
               "namespace n\n"
               "{\n"
               "  /** @uuid 11111111-1111-1111-1111-111111111111 */\n"
               "  primitive P\n"
               "  /** @uuid 22222222-2222-2222-2222-222222222222 */\n"
               "  model Base { private field P hidden }\n"
               "  /** @uuid 33333333-3333-3333-3333-333333333333 */\n"
               "  model M extends Base { entrypoint ep { in hidden } }\n"
               "}\n");
  cursiv::test::expectError(result,
                            cursiv::test::contains("The Field is not visible"));
}

// ...but a private Field declared DIRECTLY on the SAME Model is fine, since
// the reference and the Field share the same enclosing container.
TEST(XsmpcatValidator, EntryPointCanReferenceOwnPrivateField) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result =
      validate(ws,
               "catalogue c\n"
               "namespace n\n"
               "{\n"
               "  /** @uuid 11111111-1111-1111-1111-111111111111 */\n"
               "  primitive P\n"
               "  /** @uuid 22222222-2222-2222-2222-222222222222 */\n"
               "  model M { private input field P x entrypoint ep { in x } }\n"
               "}\n");
  cursiv::test::expectNoIssues(
      result, {.messageMatches = cursiv::test::contains("is not visible")});
}

// The same visibility rule applies to `Property -> attachedField`.
TEST(XsmpcatValidator, PropertyCannotAttachToPrivateFieldFromBaseModel) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(ws,
                         "catalogue c\n"
                         "namespace n\n"
                         "{\n"
                         "  /** @uuid 11111111-1111-1111-1111-111111111111 */\n"
                         "  primitive P\n"
                         "  /** @uuid 22222222-2222-2222-2222-222222222222 */\n"
                         "  model Base { private field P hidden }\n"
                         "  /** @uuid 33333333-3333-3333-3333-333333333333 */\n"
                         "  model M extends Base { property P p -> hidden }\n"
                         "}\n");
  cursiv::test::expectError(result,
                            cursiv::test::contains("The Field is not visible"));
}

// A private Type (the DEFAULT visibility, absent an explicit modifier) is not
// visible from outside its enclosing Namespace.
TEST(XsmpcatValidator, TypeNotVisibleFromOutsideItsNamespace) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(ws,
                         "catalogue c\n"
                         "namespace n1 {\n"
                         "  /** @uuid 11111111-1111-1111-1111-111111111111 */\n"
                         "  primitive P\n"
                         "}\n"
                         "namespace n2 {\n"
                         "  /** @uuid 22222222-2222-2222-2222-222222222222 */\n"
                         "  struct S { field n1.P x }\n"
                         "}\n");
  cursiv::test::expectError(result, cursiv::test::contains("is not visible"));
}

// ...but the SAME private Type is visible from within its own Namespace.
TEST(XsmpcatValidator, PrivateTypeVisibleFromWithinSameNamespace) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(ws,
                         "catalogue c\n"
                         "namespace n {\n"
                         "  /** @uuid 11111111-1111-1111-1111-111111111111 */\n"
                         "  primitive P\n"
                         "  /** @uuid 22222222-2222-2222-2222-222222222222 */\n"
                         "  struct S { field P x }\n"
                         "}\n");
  cursiv::test::expectNoIssues(
      result, {.messageMatches = cursiv::test::contains("is not visible")});
}

// An explicitly `public` Type is visible from any Namespace.
TEST(XsmpcatValidator, PublicTypeVisibleFromAnyNamespace) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(ws,
                         "catalogue c\n"
                         "namespace n1 {\n"
                         "  /** @uuid 11111111-1111-1111-1111-111111111111 */\n"
                         "  public primitive P\n"
                         "}\n"
                         "namespace n2 {\n"
                         "  /** @uuid 22222222-2222-2222-2222-222222222222 */\n"
                         "  struct S { field n1.P x }\n"
                         "}\n");
  cursiv::test::expectNoIssues(
      result, {.messageMatches = cursiv::test::contains("is not visible")});
}

// A `@deprecated` doc-tag on the referenced Type warns at the reference site.
TEST(XsmpcatValidator, DeprecatedTypeReferenceWarns) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(ws,
                         "catalogue c\n"
                         "namespace n {\n"
                         "  /**\n"
                         "   * @uuid 11111111-1111-1111-1111-111111111111\n"
                         "   * @deprecated Use Q instead.\n"
                         "   */\n"
                         "  primitive P\n"
                         "  /** @uuid 22222222-2222-2222-2222-222222222222 */\n"
                         "  struct S { field P x }\n"
                         "}\n");
  cursiv::test::expectWarning(
      result, cursiv::test::contains("Deprecated: Use Q instead."));
}

// A bare `@deprecated` tag (no explanation text) still warns, with a shorter
// message.
TEST(XsmpcatValidator, BareDeprecatedTypeReferenceWarns) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(ws,
                         "catalogue c\n"
                         "namespace n {\n"
                         "  /**\n"
                         "   * @uuid 11111111-1111-1111-1111-111111111111\n"
                         "   * @deprecated\n"
                         "   */\n"
                         "  primitive P\n"
                         "  /** @uuid 22222222-2222-2222-2222-222222222222 */\n"
                         "  struct S { field P x }\n"
                         "}\n");
  cursiv::test::expectWarning(result, cursiv::test::contains("Deprecated."));
}

// "Missing Type UUID" anchors on the type's own introducing keyword (here
// `class`), not the whole node.
TEST(XsmpcatValidator, MissingUuidAnchorsOnTypeKeyword) {
  auto ws = xsmp::xsmpcatWorkspace();
  const std::string text = "catalogue c\nnamespace n { class A {} }\n";
  auto result = validate(ws, text);
  const auto keywordBegin = text.find("class");
  ASSERT_NE(keywordBegin, std::string::npos);
  auto found = false;
  for (const auto& diagnostic : result.diagnostics) {
    if (diagnostic.message.find("Missing Type UUID") != std::string::npos) {
      found = true;
      EXPECT_EQ(diagnostic.range.begin, keywordBegin);
      EXPECT_EQ(diagnostic.range.end,
                keywordBegin + std::string_view("class").size());
    }
  }
  EXPECT_TRUE(found);
}

// Same anchoring for `exception` (a subtype of `class` in the AST): it anchors
// on its own keyword, not on `class`.
TEST(XsmpcatValidator, MissingUuidAnchorsOnExceptionKeyword) {
  auto ws = xsmp::xsmpcatWorkspace();
  const std::string text = "catalogue c\nnamespace n { exception E {} }\n";
  auto result = validate(ws, text);
  const auto keywordBegin = text.find("exception");
  ASSERT_NE(keywordBegin, std::string::npos);
  auto found = false;
  for (const auto& diagnostic : result.diagnostics) {
    if (diagnostic.message.find("Missing Type UUID") != std::string::npos) {
      found = true;
      EXPECT_EQ(diagnostic.range.begin, keywordBegin);
      EXPECT_EQ(diagnostic.range.end,
                keywordBegin + std::string_view("exception").size());
    }
  }
  EXPECT_TRUE(found);
}

// A Service with an abstract-by-attribute Operation but no `abstract` modifier
// itself warns "shall be abstract". The `Attributes.Abstract` attribute type is
// declared inline here rather than taken from the builtins.
TEST(XsmpcatValidator, ServiceMissingAbstractModifier) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(ws,
                         "catalogue c\n"
                         "namespace Attributes {\n"
                         "  primitive Flag\n"
                         "  attribute Flag Abstract = true\n"
                         "}\n"
                         "namespace n {\n"
                         "  service MyService {\n"
                         "    @Attributes.Abstract\n"
                         "    def void op()\n"
                         "  }\n"
                         "}\n");
  cursiv::test::expectWarning(
      result, cursiv::test::contains("The Service shall be abstract."));
}

// Same model, but the Service is already `abstract` — no warning.
TEST(XsmpcatValidator, AbstractServiceHasNoMissingAbstractWarning) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(ws,
                         "catalogue c\n"
                         "namespace Attributes {\n"
                         "  primitive Flag\n"
                         "  attribute Flag Abstract = true\n"
                         "}\n"
                         "namespace n {\n"
                         "  abstract service MyService {\n"
                         "    @Attributes.Abstract\n"
                         "    def void op()\n"
                         "  }\n"
                         "}\n");
  for (const auto& diagnostic : result.diagnostics) {
    EXPECT_EQ(diagnostic.message.find("shall be abstract"), std::string::npos);
  }
}

// "Duplicated Type name" is scoped by fully qualified name, not by the bare
// simple name: two types with the same simple name in different namespaces of
// the same document do not collide.
TEST(XsmpcatValidator, SameSimpleNameDifferentNamespaceIsNotDuplicated) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(ws,
                         "catalogue c\n"
                         "namespace a { primitive S }\n"
                         "namespace b { primitive S }\n");
  for (const auto& diagnostic : result.diagnostics) {
    EXPECT_EQ(diagnostic.message.find("Duplicated Type name"),
              std::string::npos);
  }
}

// Two types with the SAME fully-qualified name (here: two separate `Namespace`
// blocks reopening `n`) are genuinely duplicated.
TEST(XsmpcatValidator, SameQualifiedNameIsDuplicated) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(ws,
                         "catalogue c\n"
                         "namespace n { primitive S }\n"
                         "namespace n { primitive S }\n");
  cursiv::test::expectError(result,
                            cursiv::test::contains("Duplicated Type name"));
}

// `checkService`: two `Service` declarations sharing a name anywhere in the
// (workspace-wide, unfiltered) index conflict, even across documents.
TEST(XsmpcatValidator, DuplicatedServiceNameAcrossDocuments) {
  auto ws = xsmp::xsmpcatWorkspace();
  xsmp::parse(ws, "xsmpcat",
              "catalogue other\n"
              "namespace n\n"
              "{\n"
              "  /** @uuid 11111111-1111-1111-1111-111111111111 */\n"
              "  service Shared {}\n"
              "}\n",
              "file:///other.xsmpcat");
  auto result = validate(ws,
                         "catalogue c\n"
                         "namespace n\n"
                         "{\n"
                         "  /** @uuid 22222222-2222-2222-2222-222222222222 */\n"
                         "  service Shared {}\n"
                         "}\n");
  cursiv::test::expectError(result,
                            cursiv::test::contains("Duplicated Service name"));
}

// Two `catalogue` documents sharing a name anywhere in the (workspace-wide,
// unfiltered) index conflict.
TEST(XsmpcatValidator, DuplicatedCatalogueNameAcrossDocuments) {
  auto ws = xsmp::xsmpcatWorkspace();
  xsmp::parse(ws, "xsmpcat", "catalogue shared\n", "file:///other.xsmpcat");
  auto result = validate(ws, "catalogue shared\n");
  cursiv::test::expectError(result,
                            cursiv::test::contains("Duplicated Document name"));
}

// Two catalogues with DIFFERENT names never collide.
TEST(XsmpcatValidator, DistinctCatalogueNamesHaveNoDuplicateError) {
  auto ws = xsmp::xsmpcatWorkspace();
  xsmp::parse(ws, "xsmpcat", "catalogue other\n", "file:///other.xsmpcat");
  auto result = validate(ws, "catalogue c\n");
  cursiv::test::expectNoIssues(
      result,
      {.messageMatches = cursiv::test::contains("Duplicated Document name")});
}

// The grammar's `ID` terminal allows a leading `_` (it only rejects a leading
// digit), but a name must start with a letter.
TEST(XsmpcatValidator, NameStartingWithUnderscoreIsInvalid) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(ws, "catalogue c\nnamespace n { primitive _foo }\n");
  cursiv::test::expectError(
      result, cursiv::test::contains("A name shall start with a letter"));
}

TEST(XsmpcatValidator, CppReservedKeywordNameIsInvalid) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(ws, "catalogue c\nnamespace n { primitive int }\n");
  cursiv::test::expectError(
      result, cursiv::test::contains("shall not be an ISO/ANSI C++ keyword"));
}

// `ReturnParameter` is not a `NamedElement`, but its name reaches the generated
// C++ all the same.
TEST(XsmpcatValidator, ReturnParameterNameIsNotACppKeyword) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(ws,
                         "catalogue c\n"
                         "namespace n\n"
                         "{\n"
                         "  primitive Bool\n"
                         "  class C { def Bool private doIt() }\n"
                         "}\n");
  cursiv::test::expectError(
      result,
      cursiv::test::contains("A name shall not be an ISO/ANSI C++ keyword."));
}

// --- Constant-expression solver diagnostics ---------------------------------

// `constant` and `field` are member declarations: they are only valid inside a
// Structure/Class/Interface/Component body, never directly in a Namespace.
// `Smp.`-qualified primitives are needed wherever the declared type must be
// recognized by its well-known fully-qualified name.

TEST(XsmpcatValidator, UnknownBuiltInConstantIsInvalid) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(ws,
                         "catalogue c\n"
                         "namespace n\n"
                         "{\n"
                         "  primitive Float64\n"
                         "  struct S { constant Float64 c = $UNKNOWN }\n"
                         "}\n");
  cursiv::test::expectError(
      result, cursiv::test::contains("Unknown built-in constant"));
}

TEST(XsmpcatValidator, UnknownBuiltInFunctionIsInvalid) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(ws,
                         "catalogue c\n"
                         "namespace n\n"
                         "{\n"
                         "  primitive Float64\n"
                         "  struct S { constant Float64 c = $unknownFn(1.0) }\n"
                         "}\n");
  cursiv::test::expectError(
      result, cursiv::test::contains("Unknown built-in function"));
}

TEST(XsmpcatValidator, MissingBuiltInFunctionArgumentIsInvalid) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(ws,
                         "catalogue c\n"
                         "namespace n\n"
                         "{\n"
                         "  primitive Float64\n"
                         "  struct S { constant Float64 c = $sqrt() }\n"
                         "}\n");
  cursiv::test::expectError(result, cursiv::test::contains("Missing argument"));
}

TEST(XsmpcatValidator, ReferencingPrivateConstantFromOutsideIsNotVisible) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(ws,
                         "catalogue c\n"
                         "namespace n\n"
                         "{\n"
                         "  primitive Int32\n"
                         "  class S { private constant Int32 hidden = 1 }\n"
                         "  struct T { constant Int32 useIt = S.hidden }\n"
                         "}\n");
  cursiv::test::expectError(
      result, cursiv::test::contains("The Constant is not visible"));
}

TEST(XsmpcatValidator, PrivateConstantVisibleFromWithinSameType) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(ws,
                         "catalogue c\n"
                         "namespace n\n"
                         "{\n"
                         "  primitive Int32\n"
                         "  class S {\n"
                         "    private constant Int32 hidden = 1\n"
                         "    constant Int32 sameType = hidden\n"
                         "  }\n"
                         "}\n");
  cursiv::test::expectNoIssues(
      result, {.messageMatches = cursiv::test::contains("is not visible")});
}

TEST(XsmpcatValidator, IntegerLiteralOverflowIsInvalid) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(ws,
                         "catalogue c\n"
                         "namespace n\n"
                         "{\n"
                         "  primitive Int32\n"
                         "  struct S { constant Int32 c = "
                         "99999999999999999999999999999999999999 }\n"
                         "}\n");
  cursiv::test::expectError(result,
                            cursiv::test::contains("Conversion overflow"));
}

TEST(XsmpcatValidator, BinaryOperatorNotSupportedForStringOperandsIsInvalid) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(ws,
                         "catalogue c\n"
                         "namespace n\n"
                         "{\n"
                         "  primitive String8\n"
                         "  struct S { constant String8 c = \"a\" + \"b\" }\n"
                         "}\n");
  cursiv::test::expectError(
      result, cursiv::test::contains("is not supported for operands of type"));
}

TEST(XsmpcatValidator, UnaryOperatorNotSupportedForStringOperandIsInvalid) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(ws,
                         "catalogue c\n"
                         "namespace n\n"
                         "{\n"
                         "  primitive String8\n"
                         "  struct S { constant String8 c = -\"a\" }\n"
                         "}\n");
  cursiv::test::expectError(
      result, cursiv::test::contains("is not supported for operand of type"));
}

TEST(XsmpcatValidator, Char8MultiCharacterLiteralIsInvalid) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(ws,
                         "catalogue c\n"
                         "namespace Smp { primitive Char8 }\n"
                         "namespace n\n"
                         "{\n"
                         "  struct S { constant Smp.Char8 c = 'ab' }\n"
                         "}\n");
  cursiv::test::expectError(
      result, cursiv::test::contains("shall contain exactly one character"));
}

TEST(XsmpcatValidator, EnumLiteralOfWrongEnumTypeIsInvalid) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(ws,
                         "catalogue c\n"
                         "namespace n\n"
                         "{\n"
                         "  enum A { X = 0 }\n"
                         "  enum B { Y = 0 }\n"
                         "  struct S { constant A c = B.Y }\n"
                         "}\n");
  cursiv::test::expectError(result, cursiv::test::contains("is not of type"));
}

TEST(XsmpcatValidator, BareIntegerForEnumerationWarnsEnumLiteralExpected) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(ws,
                         "catalogue c\n"
                         "namespace n\n"
                         "{\n"
                         "  enum A { X = 0, Y = 1 }\n"
                         "  struct S { constant A c = 1 }\n"
                         "}\n");
  cursiv::test::expectWarning(
      result, cursiv::test::contains("An enumeration literal is expected"));
}

// A float converts to an integer only when it is a whole number inside the
// range of the target type; it is refused, not wrapped.
TEST(XsmpcatValidator, FloatOutsideTheIntegerTypeIsInvalid) {
  for (const auto* declaration :
       {"constant Smp.Int8 x = 300.0", "constant Smp.UInt64 x = -2.0",
        "constant Smp.Int32 x = 1e300", "constant Smp.Int32 x = 1.5"}) {
    SCOPED_TRACE(declaration);
    auto ws = xsmp::xsmpcatWorkspace();
    xsmp::loadXsmpcatBuiltins(ws.shared());
    const auto result =
        validate(ws, std::string("catalogue c namespace n { /** @uuid ")
                         + "11111111-1111-4111-8111-111111111111 */ struct S { "
                         + declaration + " } }");
    EXPECT_TRUE(std::ranges::any_of(
        result.document->diagnostics, [](const auto& diagnostic) {
          return diagnostic.severity == cursiv::DiagnosticSeverity::Error;
        }));
  }
  auto ws = xsmp::xsmpcatWorkspace();
  xsmp::loadXsmpcatBuiltins(ws.shared());
  const auto valid =
      validate(ws,
               "catalogue c namespace n { /** @uuid "
               "11111111-1111-4111-8111-111111111111 */ struct S { constant "
               "Smp.Int8 x = 100.0 } }");
  cursiv::test::expectNoIssues(valid,
                               {.severity = cursiv::DiagnosticSeverity::Error});
}

TEST(XsmpcatValidator, StringCannotBeConvertedToIntegerConstant) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(ws,
                         "catalogue c\n"
                         "namespace n\n"
                         "{\n"
                         "  primitive Int32\n"
                         "  struct S { constant Int32 c = \"abc\" }\n"
                         "}\n");
  cursiv::test::expectError(result,
                            cursiv::test::contains("cannot be converted to"));
}

// An ISO-8601-like duration string is a valid `Smp.Duration` value, as in the
// ECSS SMP builtin catalogues (e.g. `Smp.Duration cycleTime = "PT0S"`).
TEST(XsmpcatValidator, IsoDurationStringConvertsToDuration) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(ws,
                         "catalogue c\n"
                         "namespace Smp { primitive Duration }\n"
                         "namespace n\n"
                         "{\n"
                         "  struct S { constant Smp.Duration c = \"PT0S\" }\n"
                         "}\n");
  cursiv::test::expectNoIssues(
      result,
      {.messageMatches = cursiv::test::contains("cannot be converted")});
}

// --- Array/structure literal shape validation --------------------------------

TEST(XsmpcatValidator, ArrayLiteralTooManyElementsIsInvalid) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(ws,
                         "catalogue c\n"
                         "namespace n\n"
                         "{\n"
                         "  primitive Int32\n"
                         "  array Arr = Int32[2]\n"
                         "  struct S { field Arr c = {1, 2, 3} }\n"
                         "}\n");
  cursiv::test::expectError(
      result, cursiv::test::contains("expect 2 element(s), got 3 element(s)"));
}

TEST(XsmpcatValidator, ArrayLiteralPartialInitializationWarns) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(ws,
                         "catalogue c\n"
                         "namespace n\n"
                         "{\n"
                         "  primitive Int32\n"
                         "  array Arr = Int32[3]\n"
                         "  struct S { field Arr c = {1, 2} }\n"
                         "}\n");
  cursiv::test::expectWarning(result,
                              cursiv::test::contains("Partial initialization"));
}

TEST(XsmpcatValidator, ArrayLiteralCorrectSizeHasNoIssue) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(ws,
                         "catalogue c\n"
                         "namespace n\n"
                         "{\n"
                         "  primitive Int32\n"
                         "  array Arr = Int32[2]\n"
                         "  struct S { field Arr c = {1, 2} }\n"
                         "}\n");
  cursiv::test::expectNoIssues(
      result, {.messageMatches = cursiv::test::contains("element(s)")});
}

TEST(XsmpcatValidator, ArrayFieldRequiresCollectionLiteral) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(ws,
                         "catalogue c\n"
                         "namespace n\n"
                         "{\n"
                         "  primitive Int32\n"
                         "  array Arr = Int32[2]\n"
                         "  struct S { field Arr c = 5 }\n"
                         "}\n");
  cursiv::test::expectError(
      result, cursiv::test::contains(
                  "An array shall be initialized with a collection"));
}

TEST(XsmpcatValidator, StructLiteralInvalidFieldNameIsInvalid) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(ws,
                         "catalogue c\n"
                         "namespace n\n"
                         "{\n"
                         "  primitive Int32\n"
                         "  struct Point { field Int32 x field Int32 y }\n"
                         "  struct S { field Point p = { 1, .x = 2 } }\n"
                         "}\n");
  cursiv::test::expectError(
      result, cursiv::test::contains("Invalid field name, expecting y"));
}

TEST(XsmpcatValidator, StructLiteralCorrectFieldsHaveNoIssue) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(ws,
                         "catalogue c\n"
                         "namespace n\n"
                         "{\n"
                         "  primitive Int32\n"
                         "  struct Point { field Int32 x field Int32 y }\n"
                         "  struct S { field Point p = { 1, 2 } }\n"
                         "}\n");
  cursiv::test::expectNoIssues(
      result, {.messageMatches = cursiv::test::contains("Invalid field name")});
}

TEST(XsmpcatValidator, StructFieldRequiresCollectionLiteral) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(ws,
                         "catalogue c\n"
                         "namespace n\n"
                         "{\n"
                         "  primitive Int32\n"
                         "  struct Point { field Int32 x field Int32 y }\n"
                         "  struct S { field Point p = 5 }\n"
                         "}\n");
  cursiv::test::expectError(
      result, cursiv::test::contains(
                  "Structure shall be initialized with a collection"));
}

// Static/Virtual/Abstract/Constructor/ByPointer/ByReference combination-rule
// checks (`@Attributes.*` well-known usages). Each `attribute Flag X = true`
// gives a bare `@Attributes.X` usage the value `true` (matching the existing
// `ServiceMissingAbstractModifier` test's pattern).
TEST(XsmpcatValidator, PropertyStaticInInterfaceIsInvalid) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(ws,
                         "catalogue c\n"
                         "namespace Attributes {\n"
                         "  primitive Flag\n"
                         "  attribute Flag Static = true\n"
                         "}\n"
                         "namespace n {\n"
                         "  primitive P\n"
                         "  interface I {\n"
                         "    @Attributes.Static\n"
                         "    property P p\n"
                         "  }\n"
                         "}\n");
  cursiv::test::expectError(
      result, cursiv::test::contains(
                  "A Property of an Interface shall not be static."));
}

TEST(XsmpcatValidator, PropertyStaticAndVirtualIsInvalid) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(ws,
                         "catalogue c\n"
                         "namespace Attributes {\n"
                         "  primitive Flag\n"
                         "  attribute Flag Static = true\n"
                         "  attribute Flag Virtual = true\n"
                         "}\n"
                         "namespace n {\n"
                         "  primitive P\n"
                         "  service S {\n"
                         "    @Attributes.Static\n"
                         "    @Attributes.Virtual\n"
                         "    property P p\n"
                         "  }\n"
                         "}\n");
  cursiv::test::expectError(
      result, cursiv::test::contains(
                  "A Property shall not be both Static and Virtual."));
}

TEST(XsmpcatValidator, PropertyStaticWithNonStaticAttachedFieldIsInvalid) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(ws,
                         "catalogue c\n"
                         "namespace Attributes {\n"
                         "  primitive Flag\n"
                         "  attribute Flag Static = true\n"
                         "}\n"
                         "namespace n {\n"
                         "  primitive P\n"
                         "  service S {\n"
                         "    @Attributes.Static\n"
                         "    property P p -> f\n"
                         "    field P f\n"
                         "  }\n"
                         "}\n");
  cursiv::test::expectError(
      result, cursiv::test::contains("A Property shall not be Static if the "
                                     "attached field is not Static."));
}

TEST(XsmpcatValidator, PropertyByPointerAndByReferenceIsInvalid) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(ws,
                         "catalogue c\n"
                         "namespace Attributes {\n"
                         "  primitive Flag\n"
                         "  attribute Flag ByPointer = true\n"
                         "  attribute Flag ByReference = true\n"
                         "}\n"
                         "namespace n {\n"
                         "  primitive P\n"
                         "  service S {\n"
                         "    @Attributes.ByPointer\n"
                         "    @Attributes.ByReference\n"
                         "    property P p\n"
                         "  }\n"
                         "}\n");
  cursiv::test::expectError(
      result, cursiv::test::contains(
                  "A Property shall not be both ByPointer and ByReference."));
}

TEST(XsmpcatValidator, OperationStaticInInterfaceIsInvalid) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(ws,
                         "catalogue c\n"
                         "namespace Attributes {\n"
                         "  primitive Flag\n"
                         "  attribute Flag Static = true\n"
                         "}\n"
                         "namespace n {\n"
                         "  interface I {\n"
                         "    @Attributes.Static\n"
                         "    def void op()\n"
                         "  }\n"
                         "}\n");
  cursiv::test::expectError(
      result, cursiv::test::contains(
                  "An Operation of an Interface shall not be static."));
}

TEST(XsmpcatValidator, OperationStaticAndAbstractIsInvalid) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(ws,
                         "catalogue c\n"
                         "namespace Attributes {\n"
                         "  primitive Flag\n"
                         "  attribute Flag Static = true\n"
                         "  attribute Flag Abstract = true\n"
                         "}\n"
                         "namespace n {\n"
                         "  abstract service S {\n"
                         "    @Attributes.Static\n"
                         "    @Attributes.Abstract\n"
                         "    def void op()\n"
                         "  }\n"
                         "}\n");
  cursiv::test::expectError(
      result, cursiv::test::contains(
                  "An Operation shall not be both Static and Abstract."));
}

TEST(XsmpcatValidator, ConstructorShallNotBeStaticOrVirtualAndIgnoresConst) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(ws,
                         "catalogue c\n"
                         "namespace Attributes {\n"
                         "  primitive Flag\n"
                         "  attribute Flag Static = true\n"
                         "  attribute Flag Virtual = true\n"
                         "  attribute Flag Constructor = true\n"
                         "  attribute Flag Const = true\n"
                         "}\n"
                         "namespace n {\n"
                         "  service S {\n"
                         "    @Attributes.Constructor\n"
                         "    @Attributes.Static\n"
                         "    @Attributes.Virtual\n"
                         "    @Attributes.Const\n"
                         "    def void create()\n"
                         "  }\n"
                         "}\n");
  cursiv::test::expectError(
      result, cursiv::test::contains("A Constructor shall not be Static."));
  cursiv::test::expectError(
      result, cursiv::test::contains("A Constructor shall not be Virtual."));
  cursiv::test::expectWarning(
      result,
      cursiv::test::contains("Const attribute is ignored for a Constructor."));
}

TEST(XsmpcatValidator, ConstructorWithReturnParameterIsInvalid) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(ws,
                         "catalogue c\n"
                         "namespace Attributes {\n"
                         "  primitive Flag\n"
                         "  attribute Flag Constructor = true\n"
                         "}\n"
                         "namespace n {\n"
                         "  primitive P\n"
                         "  service S {\n"
                         "    @Attributes.Constructor\n"
                         "    def P create()\n"
                         "  }\n"
                         "}\n");
  cursiv::test::expectError(
      result, cursiv::test::contains(
                  "A Constructor shall not have any return parameters."));
}

TEST(XsmpcatValidator, ParameterByPointerAndByReferenceIsInvalid) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(
      ws,
      "catalogue c\n"
      "namespace Attributes {\n"
      "  primitive Flag\n"
      "  attribute Flag ByPointer = true\n"
      "  attribute Flag ByReference = true\n"
      "}\n"
      "namespace n {\n"
      "  primitive P\n"
      "  service S {\n"
      "    def void op(@Attributes.ByPointer @Attributes.ByReference in P x)\n"
      "  }\n"
      "}\n");
  cursiv::test::expectError(
      result, cursiv::test::contains(
                  "A Parameter shall not be both ByPointer and ByReference."));
}

TEST(XsmpcatValidator, ReturnParameterByPointerAndByReferenceIsInvalid) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result =
      validate(ws,
               "catalogue c\n"
               "namespace Attributes {\n"
               "  primitive Flag\n"
               "  attribute Flag ByPointer = true\n"
               "  attribute Flag ByReference = true\n"
               "}\n"
               "namespace n {\n"
               "  primitive P\n"
               "  service S {\n"
               "    def @Attributes.ByPointer @Attributes.ByReference P op()\n"
               "  }\n"
               "}\n");
  cursiv::test::expectError(
      result, cursiv::test::contains(
                  "A Parameter shall not be both ByPointer and ByReference."));
}

TEST(XsmpcatValidator, InvalidCatalogueDateWarns) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(ws, "/** @date not-a-date */\ncatalogue c\n");
  cursiv::test::expectWarning(result,
                              cursiv::test::contains("Invalid date format"));
}

// "Invalid date format" anchors on the `@date` tag's own text, not the whole
// `catalogue c` declaration.
TEST(XsmpcatValidator, InvalidCatalogueDateAnchorsOnTheDateTag) {
  auto ws = xsmp::xsmpcatWorkspace();
  const std::string text = "/** @date not-a-date */\ncatalogue c\n";
  auto result = validate(ws, text);
  const auto catalogueBegin = text.find("catalogue c");
  ASSERT_NE(catalogueBegin, std::string::npos);
  auto found = false;
  for (const auto& diagnostic : result.diagnostics) {
    if (diagnostic.message.find("Invalid date format") != std::string::npos) {
      EXPECT_LT(diagnostic.range.begin, catalogueBegin)
          << "expected the diagnostic to anchor inside the doc comment, "
             "before the `catalogue c` declaration";
      EXPECT_LT(diagnostic.range.end, catalogueBegin);
      found = true;
    }
  }
  EXPECT_TRUE(found);
}

// A bare catalogue with no owning `xsmp.project` gets the unrelated "not
// contained in a project" warning, so the assertion is scoped to errors.
TEST(XsmpcatValidator, ValidCatalogueDateHasNoWarning) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result =
      validate(ws, "/** @date 1970-01-01T00:00:00Z */\ncatalogue c\n");
  cursiv::test::expectNoIssues(result,
                               {.severity = cursiv::DiagnosticSeverity::Error});
}

TEST(XsmpcatValidator, ValidCatalogueHasNoIssues) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(ws,
                         "catalogue c\n"
                         "namespace Smp\n"
                         "{\n"
                         "  /** @uuid 11111111-1111-1111-1111-111111111111 */\n"
                         "  public primitive Int32\n"
                         "}\n"
                         "namespace n\n"
                         "{\n"
                         "  /** @uuid 22222222-2222-2222-2222-222222222222 */\n"
                         "  enum E { A = 0, B = 1 }\n"
                         "  /** @uuid 33333333-3333-3333-3333-333333333333 */\n"
                         "  struct S { field Smp.Int32 x }\n"
                         "}\n");
  cursiv::test::expectNoIssues(result,
                               {.severity = cursiv::DiagnosticSeverity::Error});
}

// --- Reference and declaration checks ----------------------------------------

// A constant's "must have a value" is only reported once its type reference has
// resolved to a SimpleType.
TEST(XsmpcatValidator, ConstantMissingValueNotReportedWhenTypeIsWrongKind) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(ws,
                         "catalogue c\n"
                         "namespace n\n"
                         "{\n"
                         "  /** @uuid 11111111-1111-1111-1111-111111111111 */\n"
                         "  class K {}\n"
                         "  struct S { constant K c }\n"
                         "}\n");
  cursiv::test::expectError(
      result,
      cursiv::test::contains("cannot be used here: expected SimpleType"));
  for (const auto& diagnostic : result.diagnostics) {
    EXPECT_EQ(diagnostic.message.find("must have a value"), std::string::npos);
  }
}

// A PrimitiveType whose name doesn't resolve to one of the 14 well-known
// `Smp.*` kinds is rejected.
TEST(XsmpcatValidator, UnsupportedPrimitiveTypeIsInvalid) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result =
      validate(ws, "catalogue c\nnamespace n { primitive NotARealKind }\n");
  cursiv::test::expectError(
      result, cursiv::test::contains("Unsupported Primitive Type"));
}

// `array A = Type[]` (size omitted) still validates the item type.
TEST(XsmpcatValidator, ArrayWithoutSizeStillChecksItemType) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(ws,
                         "catalogue c\n"
                         "namespace n\n"
                         "{\n"
                         "  /** @uuid 11111111-1111-1111-1111-111111111111 */\n"
                         "  interface I {}\n"
                         "  array A = I[]\n"
                         "}\n");
  cursiv::test::expectError(result,
                            cursiv::test::contains("Missing Array size"));
  cursiv::test::expectError(
      result,
      cursiv::test::contains("cannot be used here: expected ValueType"));
}

// A Model/Service extending a wrong-kind base that also happens to cycle back
// only reports the kind mismatch, not also "Cyclic dependency detected.".
TEST(XsmpcatValidator, ComponentWrongKindBaseDoesNotAlsoReportCyclic) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(ws,
                         "catalogue c\n"
                         "namespace n\n"
                         "{\n"
                         "  /** @uuid 11111111-1111-1111-1111-111111111111 */\n"
                         "  model M extends S {}\n"
                         "  /** @uuid 22222222-2222-2222-2222-222222222222 */\n"
                         "  service S extends M {}\n"
                         "}\n");
  for (const auto& diagnostic : result.diagnostics) {
    EXPECT_EQ(diagnostic.message.find("Cyclic dependency"), std::string::npos);
  }
  cursiv::test::expectError(
      result, cursiv::test::contains("cannot be used here: expected Model"));
  cursiv::test::expectError(
      result, cursiv::test::contains("cannot be used here: expected Service"));
}

// Every reference site the grammar narrows, with the message for a target of
// another kind.
TEST(XsmpcatValidator, NarrowedReferencesRefuseOtherKinds) {
  const struct {
    std::string_view declaration;
    std::string_view message;
  } cases[] = {
      {.declaration = "class X extends I {}",
       .message = "(Interface) cannot be used here: expected Class."},
      {.declaration = "exception X extends K {}",
       .message = "(Class) cannot be used here: expected Exception."},
      {.declaration = "interface X extends S {}",
       .message = "(Structure) cannot be used here: expected Interface."},
      {.declaration = "model X extends V {}",
       .message = "(Service) cannot be used here: expected Model."},
      {.declaration = "service X extends M {}",
       .message = "(Model) cannot be used here: expected Service."},
      {.declaration = "model X implements S {}",
       .message = "(Structure) cannot be used here: expected Interface."},
      {.declaration = "array X = I[1]",
       .message = "(Interface) cannot be used here: expected ValueType."},
      {.declaration = "using X = I*",
       .message = "(Interface) cannot be used here: expected ValueType."},
      {.declaration = "integer X extends S",
       .message = "(Structure) cannot be used here: expected PrimitiveType."},
      {.declaration = "float X extends S",
       .message = "(Structure) cannot be used here: expected PrimitiveType."},
      {.declaration = "event X extends S",
       .message = "(Structure) cannot be used here: expected SimpleType."},
      {.declaration = "attribute I X = 1",
       .message = "(Interface) cannot be used here: expected ValueType."},
      {.declaration = "class X { constant S c = 1 }",
       .message = "(Structure) cannot be used here: expected SimpleType."},
      {.declaration = "class X { field E f }",
       .message = "(EventType) cannot be used here: expected LanguageType."},
      {.declaration = "class X { property E p }",
       .message = "(EventType) cannot be used here: expected LanguageType."},
      {.declaration = "class X { property S p get throws S }",
       .message = "(Structure) cannot be used here: expected Exception."},
      {.declaration = "class X { property S p set throws S }",
       .message = "(Structure) cannot be used here: expected Exception."},
      {.declaration = "class X { def void f() throws S }",
       .message = "(Structure) cannot be used here: expected Exception."},
      {.declaration = "class X { def E f() }",
       .message = "(EventType) cannot be used here: expected LanguageType."},
      {.declaration = "class X { def void f(E p) }",
       .message = "(EventType) cannot be used here: expected LanguageType."},
      {.declaration = "class X { association E a }",
       .message = "(EventType) cannot be used here: expected LanguageType."},
      {.declaration = "model X { container S* c }",
       .message = "(Structure) cannot be used here: expected ReferenceType."},
      {.declaration = "model X { container I* c = S }",
       .message = "(Structure) cannot be used here: expected Component."},
      {.declaration = "model X { reference S* r }",
       .message = "(Structure) cannot be used here: expected ReferenceType."},
      {.declaration = "model X { eventsink S e }",
       .message = "(Structure) cannot be used here: expected EventType."},
      {.declaration = "model X { eventsource S e }",
       .message = "(Structure) cannot be used here: expected EventType."},
      {.declaration = "@S struct X {}",
       .message = "(Structure) cannot be used here: expected AttributeType."},
  };

  for (const auto& site : cases) {
    SCOPED_TRACE(site.declaration);
    auto ws = xsmp::xsmpcatWorkspace();
    auto result = validate(ws,
                           "catalogue c\nnamespace n\n{\n"
                           "  struct S {} interface I {} event E class K {}\n"
                           "  model M {} service V {}\n  "
                               + std::string(site.declaration) + "\n}\n");
    cursiv::test::expectError(
        result, cursiv::test::contains(std::string(site.message)));
  }
}

// The nearest `Same` is a structure, which `implements` does not accept, so the
// interface links. A name found nowhere is reported as unresolved.
TEST(XsmpcatValidator, NarrowedReferenceSkipsHomonymOfAnotherKind) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(ws, R"(catalogue c
namespace n
{
  /** @uuid 11111111-1111-1111-1111-111111111111 */
  interface Same {}
  namespace inner
  {
    /** @uuid 22222222-2222-2222-2222-222222222222 */
    struct Same {}
    /** @uuid 33333333-3333-3333-3333-333333333333 */
    model M implements Same { reference Missing* other }
  }
})");
  cursiv::test::expectError(
      result, cursiv::test::contains(
                  "Could not resolve reference to Type named 'Missing'."));
  for (const auto& diagnostic : result.diagnostics) {
    EXPECT_EQ(diagnostic.message.find("not a sub type"), std::string::npos)
        << diagnostic.message;
  }
}

// `E extends K` fails its own kind (`K` isn't an Exception), so that base stays
// unlinked and `K extends E` closes no cycle.
TEST(XsmpcatValidator, ClassWrongKindBaseDoesNotAlsoReportCyclic) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(ws,
                         "catalogue c\n"
                         "namespace n\n"
                         "{\n"
                         "  /** @uuid 11111111-1111-1111-1111-111111111111 */\n"
                         "  exception E extends K {}\n"
                         "  /** @uuid 22222222-2222-2222-2222-222222222222 */\n"
                         "  class K extends E {}\n"
                         "}\n");
  auto cyclicCount = 0;
  for (const auto& diagnostic : result.diagnostics) {
    if (diagnostic.message.find("Cyclic dependency") != std::string::npos) {
      ++cyclicCount;
    }
  }
  EXPECT_EQ(cyclicCount, 0);
  cursiv::test::expectError(
      result,
      cursiv::test::contains("cannot be used here: expected Exception"));
}

// `Container.defaultComponent` is only checked once `Container.type` itself
// resolved to a valid kind.
TEST(XsmpcatValidator, ContainerDefaultComponentNotCheckedWhenTypeIsWrongKind) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(ws,
                         "catalogue c\n"
                         "namespace n\n"
                         "{\n"
                         "  struct WrongKind {}\n"
                         "  /**\n"
                         "   * @uuid 11111111-1111-1111-1111-111111111111\n"
                         "   * @deprecated old\n"
                         "   */\n"
                         "  model M2 {}\n"
                         "  model S { container WrongKind items = M2 }\n"
                         "}\n");
  cursiv::test::expectError(
      result, cursiv::test::contains(
                  "(Structure) cannot be used here: expected ReferenceType."));
  for (const auto& diagnostic : result.diagnostics) {
    EXPECT_EQ(diagnostic.message.find("Deprecated"), std::string::npos);
  }
}

// `AttributeType.default_` is only checked once `AttributeType.type` itself
// resolved to a ValueType.
TEST(XsmpcatValidator, AttributeTypeDefaultNotCheckedWhenTypeIsWrongKind) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(ws,
                         "catalogue c\n"
                         "namespace n\n"
                         "{\n"
                         "  /** @uuid 11111111-1111-1111-1111-111111111111 */\n"
                         "  interface I {}\n"
                         "  /** @uuid 22222222-2222-2222-2222-222222222222 */\n"
                         "  attribute I myAttr = 5\n"
                         "}\n");
  cursiv::test::expectError(
      result,
      cursiv::test::contains("cannot be used here: expected ValueType"));
  for (const auto& diagnostic : result.diagnostics) {
    EXPECT_EQ(diagnostic.message.find("cannot be converted"),
              std::string::npos);
  }
}

// An `AttributeType`'s `@usage` tag naming an unrecognized category warns.
TEST(XsmpcatValidator, AttributeTypeUsageInvalidCategoryWarns) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(ws,
                         "catalogue c\n"
                         "namespace n\n"
                         "{\n"
                         "  public primitive Flag\n"
                         "  /**\n"
                         "   * @usage NotACategory\n"
                         "   */\n"
                         "  public attribute Flag Foo = true\n"
                         "}\n");
  cursiv::test::expectWarning(result, cursiv::test::contains("Unknown usage"));
}

// Repeating the same `@usage` category on an `AttributeType` warns once per
// repeat.
TEST(XsmpcatValidator, AttributeTypeUsageDuplicatedCategoryWarns) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(ws,
                         "catalogue c\n"
                         "namespace n\n"
                         "{\n"
                         "  public primitive Flag\n"
                         "  /**\n"
                         "   * @usage Class\n"
                         "   * @usage Class\n"
                         "   */\n"
                         "  public attribute Flag Foo = true\n"
                         "}\n");
  cursiv::test::expectWarning(result,
                              cursiv::test::contains("Duplicated usage"));
}

// Applying an `@Foo` instance whose AttributeType restricts `@usage` to a
// category the annotated element doesn't match warns (distinct from the
// declaration-side check above).
TEST(XsmpcatValidator, AttributeUsageMismatchedElementKindWarns) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(ws,
                         "catalogue c\n"
                         "namespace n\n"
                         "{\n"
                         "  public primitive Flag\n"
                         "  /**\n"
                         "   * @usage Class\n"
                         "   */\n"
                         "  public attribute Flag Foo = true\n"
                         "  @Foo\n"
                         "  public primitive P\n"
                         "}\n");
  cursiv::test::expectWarning(
      result,
      cursiv::test::contains("disallowed for element of type PrimitiveType"));
}

// An `@Foo` instance whose AttributeType's `@usage` matches the annotated
// element's own kind is legal.
TEST(XsmpcatValidator,
     AttributeUsageMatchingElementKindHasNoDisallowedWarning) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(ws,
                         "catalogue c\n"
                         "namespace n\n"
                         "{\n"
                         "  public primitive Flag\n"
                         "  /**\n"
                         "   * @usage PrimitiveType\n"
                         "   */\n"
                         "  public attribute Flag Foo = true\n"
                         "  @Foo\n"
                         "  public primitive P\n"
                         "}\n");
  for (const auto& diagnostic : result.diagnostics) {
    EXPECT_EQ(diagnostic.message.find("disallowed"), std::string::npos);
  }
}

// AST names and their reflected bases must agree for both validation and
// attribute completion, including the ReferenceType union.
TEST(XsmpcatValidator, AttributeUsageAcceptsReflectedTypeAncestors) {
  for (const auto& [usage, declaration] :
       {std::pair{"ValueType", "array A = Flag[2]"},
        std::pair{"SimpleType", "string S[8]"},
        std::pair{"ReferenceType", "interface I {}"},
        std::pair{"ReferenceType", "model M {}"}}) {
    SCOPED_TRACE(usage);
    SCOPED_TRACE(declaration);
    auto ws = xsmp::xsmpcatWorkspace();
    const auto result = validate(
        ws, "catalogue c namespace n { public primitive Flag /** @usage "
                + std::string(usage) + " */ public attribute Flag Foo = true "
                + "@Foo public " + declaration + " }");
    ASSERT_TRUE(result.document->parseSucceeded());
    for (const auto& diagnostic : result.diagnostics)
      EXPECT_EQ(diagnostic.message.find("disallowed"), std::string::npos);
  }
}

// Applying a non-`@allowMultiple` AttributeType twice on the same element
// warns.
TEST(XsmpcatValidator, DuplicatedNonRepeatableAttributeInstanceWarns) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(ws,
                         "catalogue c\n"
                         "namespace n\n"
                         "{\n"
                         "  public primitive Flag\n"
                         "  public attribute Flag Foo = true\n"
                         "  @Foo\n"
                         "  @Foo\n"
                         "  public primitive P\n"
                         "}\n");
  cursiv::test::expectWarning(
      result,
      cursiv::test::contains("Duplicated annotation of a non-repeatable type"));
}

// `@allowMultiple` on the AttributeType suppresses the duplicate-instance
// warning above.
TEST(XsmpcatValidator, AllowMultipleAttributeInstanceHasNoDuplicateWarning) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(ws,
                         "catalogue c\n"
                         "namespace n\n"
                         "{\n"
                         "  public primitive Flag\n"
                         "  /**\n"
                         "   * @allowMultiple\n"
                         "   */\n"
                         "  public attribute Flag Foo = true\n"
                         "  @Foo\n"
                         "  @Foo\n"
                         "  public primitive P\n"
                         "}\n");
  for (const auto& diagnostic : result.diagnostics) {
    EXPECT_EQ(diagnostic.message.find("Duplicated annotation"),
              std::string::npos);
  }
}

// `Property.type` only needs to be a LanguageType, not narrowly a ValueType: a
// Property typed as an Interface/Component is legal.
TEST(XsmpcatValidator, PropertyAcceptsNonValueTypeLanguageType) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(ws,
                         "catalogue c\n"
                         "namespace n\n"
                         "{\n"
                         "  /** @uuid 11111111-1111-1111-1111-111111111111 */\n"
                         "  interface I {}\n"
                         "  /** @uuid 22222222-2222-2222-2222-222222222222 */\n"
                         "  service S { property I p }\n"
                         "}\n");
  for (const auto& diagnostic : result.diagnostics) {
    EXPECT_EQ(diagnostic.message.find("Expected a"), std::string::npos);
  }
}

// A `@deprecated` Field referenced as a Property's attachedField warns.
TEST(XsmpcatValidator, DeprecatedAttachedFieldWarns) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(ws,
                         "catalogue c\n"
                         "namespace n\n"
                         "{\n"
                         "  /** @uuid 11111111-1111-1111-1111-111111111111 */\n"
                         "  primitive P\n"
                         "  /** @uuid 22222222-2222-2222-2222-222222222222 */\n"
                         "  model M {\n"
                         "    /** @deprecated use something else */\n"
                         "    field P f\n"
                         "    property P p -> f\n"
                         "  }\n"
                         "}\n");
  cursiv::test::expectWarning(
      result, cursiv::test::contains("Deprecated: use something else"));
}

// Same for `EntryPoint.input`/`output`.
TEST(XsmpcatValidator, DeprecatedEntryPointFieldWarns) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(ws,
                         "catalogue c\n"
                         "namespace n\n"
                         "{\n"
                         "  /** @uuid 11111111-1111-1111-1111-111111111111 */\n"
                         "  primitive P\n"
                         "  /** @uuid 22222222-2222-2222-2222-222222222222 */\n"
                         "  model M {\n"
                         "    /** @deprecated use something else */\n"
                         "    input field P f\n"
                         "    entrypoint ep { in f }\n"
                         "  }\n"
                         "}\n");
  cursiv::test::expectWarning(
      result, cursiv::test::contains("Deprecated: use something else"));
}

// A duplicated type name is reported once, on the Type, not also on the
// Namespace.
TEST(XsmpcatValidator, NamespaceDoesNotDoubleReportDuplicateTypeNames) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(ws,
                         "catalogue c\n"
                         "namespace n\n"
                         "{\n"
                         "  /** @uuid 11111111-1111-1111-1111-111111111111 */\n"
                         "  struct S {}\n"
                         "  /** @uuid 22222222-2222-2222-2222-222222222222 */\n"
                         "  struct S {}\n"
                         "}\n");
  auto duplicatedNameCount = 0;
  for (const auto& diagnostic : result.diagnostics) {
    if (diagnostic.message == "Duplicated name.") {
      ++duplicatedNameCount;
    }
  }
  EXPECT_EQ(duplicatedNameCount, 0);
  cursiv::test::expectError(result,
                            cursiv::test::contains("Duplicated Type name"));
}

// The same namespace reopened twice in one document is a real collision; both
// occurrences are flagged.
TEST(XsmpcatValidator, NamespaceReopenedInSameDocumentIsDuplicated) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(ws, "catalogue c\nnamespace n {}\nnamespace n {}\n");
  auto duplicatedNameCount = 0;
  for (const auto& diagnostic : result.diagnostics) {
    if (diagnostic.message == "Duplicated name.") {
      ++duplicatedNameCount;
    }
  }
  EXPECT_EQ(duplicatedNameCount, 2);
}

// The same-document reopening check also fires for a NESTED namespace, not
// just a top-level one.
TEST(XsmpcatValidator, NestedNamespaceReopeningInSameDocumentIsDuplicated) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(ws,
                         "catalogue c\nnamespace outer { namespace inner {} "
                         "namespace inner {} }\n");
  cursiv::test::expectError(result, cursiv::test::contains("Duplicated name."));
}

// Two namespaces with DIFFERENT (fully-qualified) names never collide.
TEST(XsmpcatValidator, NamespacesWithDifferentNamesAreNotDuplicated) {
  auto ws = xsmp::xsmpcatWorkspace();
  auto result = validate(ws, "catalogue c\nnamespace a {}\nnamespace b {}\n");
  cursiv::test::expectNoIssues(
      result, {.messageMatches = cursiv::test::contains("Duplicated name.")});
}

// A "not visible" error on a reference list (here the 2nd entry of
// `Interface.base`) anchors on that specific base, not the whole `Interface`
// declaration.
TEST(XsmpcatValidator, InterfaceBaseNotVisibleAnchorsOnThatBase) {
  auto ws = xsmp::xsmpcatWorkspace();
  const std::string text =
      "catalogue c\n"
      "namespace other\n"
      "{\n"
      "  /** @uuid 22222222-2222-2222-2222-222222222222 */\n"
      "  interface IPrivate {}\n"
      "}\n"
      "namespace n\n"
      "{\n"
      "  /** @uuid 11111111-1111-1111-1111-111111111111 */\n"
      "  public interface IA {}\n"
      "  /** @uuid 33333333-3333-3333-3333-333333333333 */\n"
      "  interface IB extends IA, other.IPrivate {}\n"
      "}\n";
  auto result = validate(ws, text);
  const auto expectedBegin = text.find("other.IPrivate");
  ASSERT_NE(expectedBegin, std::string::npos);
  auto found = false;
  for (const auto& diagnostic : result.diagnostics) {
    if (diagnostic.message.find("is not visible") != std::string::npos) {
      EXPECT_EQ(diagnostic.range.begin, expectedBegin);
      found = true;
    }
  }
  EXPECT_TRUE(found);
}

TEST(XsmpcatValidator, DirectAndArrayString8FieldsAreRejectedInComponents) {
  auto ws = xsmp::xsmpcatWorkspace();
  xsmp::loadXsmpcatBuiltins(ws.shared<xsmp::XsmpSharedServices>());
  auto result = validate(ws, R"(catalogue StringFields
namespace demo {
 public array Messages = Smp.String8[4]
 public model Root { field Smp.String8 direct field Messages messages }
 public service Store { field Smp.String8 direct }
})");
  const auto count =
      std::ranges::count_if(result.document->diagnostics, [](const auto& d) {
        return d.message ==
               "A Field of a Model or Service shall not use the String8 type.";
      });
  EXPECT_EQ(count, 3);
}

TEST(XsmpcatValidator,
     SharedNestedString8FieldsAreNotLostAcrossSiblingTraversals) {
  auto ws = xsmp::xsmpcatWorkspace();
  xsmp::loadXsmpcatBuiltins(ws.shared<xsmp::XsmpSharedServices>());
  auto result = validate(ws, R"(catalogue DiamondStrings
namespace demo {
 public struct Packet { field Smp.String8 label }
 public struct WrapperB { field Packet viaB }
 public struct WrapperC { field Packet viaC }
 public struct Outer { field WrapperB b field WrapperC c }
 public model Root { field Outer wide field WrapperC viaCDirect field Packet direct }
})");
  const auto count =
      std::ranges::count_if(result.document->diagnostics, [](const auto& d) {
        return d.message ==
               "A Field of a Model or Service shall not use a type containing "
               "a String8 field.";
      });
  EXPECT_EQ(count, 3);
  for (const auto& d : result.document->diagnostics)
    if (d.message ==
        "A Field of a Model or Service shall not use a type containing a "
        "String8 field.") {
      ASSERT_EQ(d.relatedInformation.size(), 1u);
      EXPECT_EQ(d.relatedInformation[0].message,
                "String8 field demo.Packet.label");
    }
}

TEST(XsmpcatValidator,
     CompositeString8FieldsProduceDirectAndTransitiveWarnings) {
  auto ws = xsmp::xsmpcatWorkspace();
  xsmp::loadXsmpcatBuiltins(ws.shared<xsmp::XsmpSharedServices>());
  auto result = validate(ws, R"(catalogue CompositeStrings
namespace demo {
 public array Messages = Smp.String8[4]
 public struct Packet { field Smp.String8 label }
 public class Text { field Smp.String8 text }
 public struct Envelope { field Packet packet field Messages messages }
})");
  EXPECT_EQ(std::ranges::count_if(result.document->diagnostics,
                                  [](const auto& d) {
                                    return d.message ==
                             "Field type is String8. This is allowed outside "
                             "Model/Service but is not SMP compatible." &&
                         d.severity == cursiv::DiagnosticSeverity::Warning;
                                  }),
            3);
  cursiv::test::expectWarning(
      result, cursiv::test::contains("Field type contains a String8 field."));
}

TEST(XsmpcatValidator,
     BoundedStringsAndStaticString8FieldsRemainUsableInComponents) {
  auto ws = xsmp::xsmpcatWorkspace();
  xsmp::loadXsmpcatBuiltins(ws.shared<xsmp::XsmpSharedServices>());
  auto result = validate(ws, R"(catalogue SafeStrings
namespace demo {
 public string Label[16]
 public struct Data { @Static field Smp.String8 ignored field Label label }
 public model Root { field Label label field Data data }
})");
  cursiv::test::expectNoIssues(
      result, {.messageMatches =
                   cursiv::test::contains("shall not use the String8 type")});
  cursiv::test::expectNoIssues(
      result,
      {.messageMatches = cursiv::test::contains("containing a String8 field")});
}

TEST(XsmpcatValidator, FixtureDiagnosticsMatchTheExpectedMessagesAndRanges) {
  auto ws = xsmp::xsmpcatWorkspace();
  xsmp::loadXsmpcatBuiltins(ws.shared<xsmp::XsmpSharedServices>());
  std::ifstream input(std::string(XSMP_FIXTURES_DIR) + "/core/test.xsmpcat");
  ASSERT_TRUE(input);
  std::ostringstream source;
  source << input.rdbuf();
  auto result = validate(ws, source.str());
  ASSERT_TRUE(result.document->parseSucceeded());
  std::vector<std::string> actual;
  for (const auto& diagnostic : result.document->diagnostics) {
    const auto begin =
        result.document->textDocument().positionAt(diagnostic.range.begin);
    const auto end =
        result.document->textDocument().positionAt(diagnostic.range.end);
    actual.push_back(
        "[" + std::to_string(begin.line) + ":" + std::to_string(begin.character)
        + ".." + std::to_string(end.line) + ":" + std::to_string(end.character)
        + "]: " + diagnostic.message);
  }
  std::ifstream baseline(std::string(XSMP_FIXTURES_DIR)
                         + "/core/xsmpcat-validation.expected.txt");
  ASSERT_TRUE(baseline);
  std::vector<std::string> expected;
  for (std::string line; std::getline(baseline, line);) {
    if (line.starts_with('['))
      expected.push_back(std::move(line));
    else if (!expected.empty())
      expected.back() += '\n' + line;
  }
  // Every message and range is compared, duplicates included, whatever the
  // order of the diagnostics.
  std::ranges::sort(actual);
  std::ranges::sort(expected);
  EXPECT_EQ(actual, expected);
}

TEST(XsmpcatValidator,
     AnnotationUsageValuesAreValidatedAgainstTheirDeclaredType) {
  auto ws = xsmp::xsmpcatWorkspace();
  xsmp::loadXsmpcatBuiltins(ws.shared<xsmp::XsmpSharedServices>());
  auto result = validate(ws, R"(catalogue AnnotationValues namespace n {
 public attribute Smp.UInt8 Byte
 @Byte(300) public struct Invalid {}
 @Byte(7) public struct Valid {}
 @Byte public struct Missing {}
})");
  cursiv::test::expectError(
      result, cursiv::test::contains("is out of range for Smp.UInt8"));
  cursiv::test::expectError(result,
                            cursiv::test::contains("A value is required."));
}

TEST(XsmpcatValidator,
     DocumentationContentRangesRemainExactAfterUtf8Characters) {
  auto ws = xsmp::xsmpcatWorkspace();
  const std::string text =
      "catalogue Utf8\nnamespace n { /* é */ /** @uuid é-invalid */ struct S "
      "{} }\n";
  auto result = validate(ws, text);
  const auto begin = text.find("é-invalid");
  auto found = false;
  for (const auto& diagnostic : result.document->diagnostics)
    if (diagnostic.message == "The UUID is invalid.") {
      EXPECT_EQ(diagnostic.range.begin, begin);
      EXPECT_EQ(diagnostic.range.end, begin + std::string("é-invalid").size());
      found = true;
    }
  EXPECT_TRUE(found);
}

TEST(XsmpcatValidator, ParameterQualifiersDistinguishOperationOverloads) {
  auto ws = xsmp::xsmpcatWorkspace();
  xsmp::loadXsmpcatBuiltins(ws.shared<xsmp::XsmpSharedServices>());
  auto result = validate(ws, R"(catalogue Signatures namespace n {
 public class Distinct {
  def void op(in Smp.Int32 value)
  def void op(@ByPointer in Smp.Int32 value)
  def void op(@ByReference in Smp.Int32 value)
  def void op(@Const in Smp.Int32 value)
 }
 public class Duplicate {
  def void repeated(@ByPointer in Smp.Int32 first)
  def void repeated(@ByPointer in Smp.Int32 second)
 }
})");
  EXPECT_EQ(
      std::ranges::count_if(result.document->diagnostics,
                            [](const auto& d) {
                              return d.message.starts_with("Duplicated name");
                            }),
      1);
}

TEST(XsmpcatValidator, CatalogueDateMetadataIsAnXsdDateTime) {
  for (auto date :
       {"2020", "2020-05", "2020-02-30T00:00:00Z", "2020-01-01T25:00:00Z",
        "2020-01-01T00:00:00+25:00", "Fri, 21 Nov 1997 09:55:06 GMT"}) {
    auto ws = xsmp::xsmpcatWorkspace();
    const auto result =
        validate(ws, "/** @date " + std::string(date) + " */\ncatalogue Dates");
    cursiv::test::expectWarning(result,
                                cursiv::test::contains("Invalid date format"));
  }
  for (auto date : {"2020-02-29T00:00:00Z", "2020-01-01T24:00:00Z"}) {
    auto ws = xsmp::xsmpcatWorkspace();
    const auto result =
        validate(ws, "/** @date " + std::string(date) + " */\ncatalogue Dates");
    cursiv::test::expectNoIssues(
        result,
        {.messageMatches = cursiv::test::contains("Invalid date format")});
  }
}

// An attribute applies to an array or a string through a parent usage. The port
// refuses a user-declared `primitive Int32`, hence only these messages count.
TEST(XsmpcatValidator, AttributeUsagesApplyToArraysAndStringsThroughAParent) {
  auto ws = workspaceWithBuiltins();
  const auto result = validate(ws, R"(
    catalogue AttributeUsage

    namespace usage
    {
        /** @uuid 5f4d3b0e-6c1a-4f0e-9a57-2d1c8e7b6a10 */
        public primitive Int32

        /** @uuid 7a2e9c44-3b8d-4d21-8f60-1e5b7c9d0a21 */
        public primitive Bool

        /**
         * @uuid 1c9f6e2b-8a47-4c35-b1d2-6f3e0a7d5c32
         * @usage ValueType
         */
        public attribute Bool OnValueType = true

        /**
         * @uuid 2d0a7f3c-9b58-4d46-a2e3-7a4f1b8e6d43
         * @usage Array
         */
        public attribute Bool OnArray = true

        /**
         * @uuid 3e1b8a4d-0c69-4e57-b3f4-8b5a2c9f7e54
         * @usage String
         */
        public attribute Bool OnString = true

        /** @uuid 4f2c9b5e-1d7a-4f68-a4a5-9c6b3d0a8f65 */
        @OnValueType
        @OnArray
        public array Values = usage.Int32[4]

        /** @uuid 503dac6f-2e8b-4079-b5b6-ad7c4e1b9a76 */
        @OnValueType
        @OnString
        public string Text[8]

        /** @uuid 614ebd70-3f9c-418a-a6c7-be8d5f2cab87 */
        @OnArray
        public string Misplaced[8]
    }
  )");
  std::vector<std::string> disallowed;
  for (const auto& diagnostic : result.document->diagnostics)
    if (diagnostic.message.find("disallowed") != std::string::npos)
      disallowed.push_back(diagnostic.message);
  EXPECT_EQ(disallowed,
            std::vector<std::string>{
                "This annotation is disallowed for element of type String."});
}

// Any character is a valid value, whatever XML can hold.
TEST(XsmpcatValidator, LiteralsHoldAnyCharacter) {
  auto ws = workspaceWithBuiltins();
  const auto result = validate(ws, R"(catalogue C namespace n {
    /** @uuid 11111111-1111-1111-1111-111111111111 */
    public struct S {
      constant Smp.Char8 a = '\0'
      constant Smp.Char8 b = '\x01'
      constant Smp.String8 c = "a\x01"
      constant Smp.String8 d = "a" "\x1F"
      constant Smp.String8 e = "a\0b"
      constant Smp.String8 f = "\uFFFE"
      constant Smp.String8 g = "\uFFFF"
      constant Smp.String8 h = "tab\t and line\n and \u00E9"
    }
  }
)");
  std::vector<std::string> errors;
  for (const auto& diagnostic : result.document->diagnostics)
    if (diagnostic.severity == cursiv::DiagnosticSeverity::Error)
      errors.push_back(diagnostic.message);
  EXPECT_EQ(errors, std::vector<std::string>{});
}

namespace {

// The related messages of the diagnostics with `message`, in order.
std::vector<std::string> relatedOf(const cursiv::Document& document,
                                   std::string_view message) {
  std::vector<std::string> related;
  for (const auto& d : document.diagnostics)
    if (d.message == message)
      related.push_back(d.relatedInformation.empty()
                            ? std::string{}
                            : d.relatedInformation.front().message);
  return related;
}

constexpr std::string_view kNonValue =
    "A Field of a Model or Service shall not use a type containing "
    "non-ValueType fields.";
constexpr std::string_view kString8 =
    "A Field of a Model or Service shall not use a type containing a String8 "
    "field.";

}  // namespace

// The fields of each type are read once per state of the workspace, and give
// what a walk from the field finds first: through a long chain, around a cycle,
// and through a base.
TEST(XsmpcatValidator, FieldSummariesFollowChainsCyclesAndBases) {
  auto ws = workspaceWithBuiltins();
  std::string arrays;
  for (auto i = 0; i < 1500; ++i)
    arrays += "public array A" + std::to_string(i) + " = "
              + (i ? "A" + std::to_string(i - 1) : std::string("Smp.String8"))
              + "[1]\n";
  const auto chain = validate(
      ws, "catalogue c\nnamespace n {\n" + arrays
              + "public struct Holder { field A1499 value }\n"
                "public model M { field A1499 direct field Holder held }\n}\n");
  EXPECT_EQ(relatedOf(*chain.document,
                      "A Field of a Model or Service shall not use the String8 "
                      "type.")
                .size(),
            1u);
  EXPECT_EQ(relatedOf(*chain.document, kString8),
            std::vector<std::string>{"String8 field n.Holder.value"});

  auto ws2 = workspaceWithBuiltins();
  const auto cycle = validate(ws2, R"(catalogue c
namespace n {
  public interface I {}
  public struct A { field B next field Smp.String8 fromA field I objectA }
  public struct B { field A back field Smp.String8 fromB field I objectB }
  public model M { field A a field B b }
}
)");
  EXPECT_EQ(relatedOf(*cycle.document, "Recursive Field Type.").size(), 2u);
  EXPECT_EQ(relatedOf(*cycle.document, kNonValue),
            (std::vector<std::string>{"Non-ValueType field n.B.objectB",
                                      "Non-ValueType field n.A.objectA"}));
  EXPECT_EQ(relatedOf(*cycle.document, kString8),
            (std::vector<std::string>{"String8 field n.B.fromB",
                                      "String8 field n.A.fromA"}));

  auto ws3 = workspaceWithBuiltins();
  const auto bases = validate(ws3, R"(catalogue c
namespace n {
  public interface I {}
  public class Base { private field I inherited @Static public field Smp.String8 ignored }
  public class Derived extends Base { public field Smp.Int32 own }
  public model M { field Derived d }
}
)");
  EXPECT_EQ(relatedOf(*bases.document, kNonValue),
            std::vector<std::string>{"Non-ValueType field n.Base.inherited"});
  EXPECT_TRUE(relatedOf(*bases.document, kString8).empty());
}

// An edit is read again: nothing of the previous text stays.
TEST(XsmpcatValidator, FieldSummariesFollowAnEdit) {
  auto ws = workspaceWithBuiltins();
  auto& shared = ws.shared<xsmp::XsmpSharedServices>();
  const auto text = [](std::string_view leaf) {
    return "catalogue c\nnamespace n {\n  public struct Leaf { field "
           + std::string(leaf)
           + " value }\n  public struct Wrapper { field Leaf leaf }\n  public "
             "model M { field Wrapper wrapped }\n}\n";
  };
  const auto document = validate(ws, text("Smp.Int32")).document;
  EXPECT_TRUE(relatedOf(*document, kString8).empty());
  const auto edit = [&](std::string_view leaf, int version) {
    document->isOpen = true;
    shared.documentFactory->attachTextDocument(
        *document,
        cursiv::TextDocument::create(std::string(document->uri), "xsmpcat",
                                     version, text(leaf)));
    const std::string_view changed[] = {document->uri};
    shared.documentBuilder->update(changed, {});
  };
  edit("Smp.String8", 2);
  EXPECT_EQ(relatedOf(*document, kString8),
            std::vector<std::string>{"String8 field n.Leaf.value"});
  edit("Wrapper", 3);
  EXPECT_EQ(relatedOf(*document, "Recursive Field Type.").size(), 2u);
  EXPECT_TRUE(relatedOf(*document, kString8).empty());
}
