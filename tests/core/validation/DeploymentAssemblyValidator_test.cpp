#include <gtest/gtest.h>

#include <algorithm>
#include <xsmp/ast.hpp>
#include <xsmp/utils/TreeUtils.hpp>

#include "core/validation/DeploymentSemanticsFixture.hpp"

using DeploymentAssemblyValidatorTest = DeploymentSemantics;

// A name that is a C++ keyword still works in a simulator, so it only warns:
// as written, once expanded, and in the child of an instantiated assembly.
TEST_F(DeploymentAssemblyValidatorTest, ACppKeywordAsAnInstanceNameIsAWarning) {
  const auto warns = [](const cursiv::Document& document,
                        std::string_view message) {
    return std::ranges::any_of(document.diagnostics, [&](const auto& d) {
      return d.severity == cursiv::DiagnosticSeverity::Warning
             && d.message == message;
    });
  };
  auto written = build("xsmpasb", "assembly Written\nclass: demo.Root { }\n");
  EXPECT_TRUE(warns(*written, "A name shall not be an ISO/ANSI C++ keyword."));
  auto pattern = build(
      "xsmpasb", "assembly <P = \"class\"> Pattern\n{P}: demo.Root { }\n");
  EXPECT_TRUE(
      warns(*pattern,
            "The expanded name 'class' shall not be an ISO/ANSI C++ keyword."));
  auto child = build("xsmpasb", R"(assembly <P = "ok"> Inner
Root: demo.Root { child += {P}: demo.Child }
)");
  auto outer = build("xsmpasb", R"(assembly Outer
Root: demo.Root { child += I: Inner<P = "class"> }
)");
  EXPECT_FALSE(warns(*child,
                     "The expanded name 'class' shall not be an "
                     "ISO/ANSI C++ keyword."));
  EXPECT_TRUE(warns(*outer,
                    "The expanded child instance name 'class' shall not be an "
                    "ISO/ANSI C++ keyword."));
}

TEST_F(DeploymentAssemblyValidatorTest,
       AssemblyChecksSubscriptionsWritablePropertiesAndOperationParameters) {
  auto document = build("xsmpasb", R"(assembly Invocations
Root: demo.Root {
 subscribe writable -> "Boot"
 call apply(missing = 1i32, count = true)
 property readOnlyFlag = true
 property writable = 1i32
})");
  EXPECT_TRUE(
      has(*document, "selected entry point shall resolve to an EntryPoint"));
  EXPECT_TRUE(has(
      *document,
      "parameter 'missing' shall resolve to a Parameter of operation apply"));
  EXPECT_TRUE(has(*document, "compatible with type Smp.Int32"));
  EXPECT_TRUE(has(*document, "PropertyValue shall target a writable Property"));
  EXPECT_TRUE(has(*document, "compatible with type Smp.Bool"));
}

namespace {

constexpr auto kLinkedTypes = R"(catalogue Linked namespace linked {
 public event EvA extends Smp.Int32
 public event EvB extends Smp.Float64
 public array Pair = Smp.Int32[2]
 public array OtherPair = Smp.Int32[2]
 public array Triple = Smp.Int32[3]
 public string Short[2]
 public string OtherShort[2]
 public string Long[3]
 public enum E { A = 0 }
 public enum OtherE { B = 0 }
 public struct S { output field Smp.Int32 i field Smp.Bool b }
 public struct OtherS { field Smp.Int32 j field Smp.Bool c }
 public struct Swapped { field Smp.Bool b field Smp.Int32 i }
 public class C { public field Smp.Int32 i }
 public class OtherC { public field Smp.Int32 i }
 public model M {
   eventsource EvA srcA
   eventsink EvA sinkA
   eventsink EvB sinkB
   output field Smp.Int32 oi input field Smp.Int32 ii
   input field Smp.Float64 if64 input field Smp.Int16 ii16
   input field Smp.UInt32 iu32
   output field Smp.Duration oDuration input field Smp.Int64 ii64
   input field Smp.DateTime iDateTime
   output field Pair oPair input field OtherPair iPair input field Triple iTriple
   output field Short oShort input field OtherShort iShort input field Long iLong
   output field E oE input field E iE input field OtherE iOtherE
   output field S oS input field OtherS iS input field Swapped iSwapped
   output field C oC input field C iC input field OtherC iOtherC
 }
})";

}  // namespace

// ECSS-E-ST-40-08 5.2.7.3 and 5.2.7.4f, with the equivalences of
// ECSS-E-ST-40-07 Table 5-4.
TEST_F(DeploymentAssemblyValidatorTest, LinksConnectCompatibleEnds) {
  add("src/linked.xsmpcat", kLinkedTypes);
  auto link = [&, index = 0](std::string_view text) mutable {
    return build("xsmpasb", "assembly Links" + std::to_string(index++)
                                + " R: linked.M { " + std::string(text)
                                + " }\n");
  };
  clean(*link("event link srcA -> sinkA"));
  EXPECT_TRUE(
      has(*link("event link srcA -> sinkB"), "shall have the same Event Type"));
  for (const auto* good :
       {"oi -> ii", "oDuration -> ii64", "oPair -> iPair", "oPair[0] -> ii",
        "oShort -> iShort", "oE -> iE", "oS -> iS", "oC -> iC", "oS.i -> ii"})
    clean(*link("field link " + std::string(good)));
  for (const auto* bad :
       {"oi -> if64", "oi -> ii16", "oi -> iu32", "oDuration -> iDateTime",
        "oPair -> iTriple", "oShort -> iLong", "oE -> iOtherE",
        "oS -> iSwapped", "oC -> iOtherC"})
    EXPECT_TRUE(has(*link("field link " + std::string(bad)),
                    "same or a semantically equivalent type"))
        << bad;
  // An unsafe path asks not to be checked.
  EXPECT_FALSE(has(*link("field link oi -> unsafe if64"),
                   "semantically equivalent type"));
}

TEST_F(DeploymentAssemblyValidatorTest, TemplateArgumentsFitTheirParameters) {
  add("src/sub.xsmpasb",
      "assembly <A = \"x\", N: int32 = 1> SubAsm\nR{A}{N}: demo.Child\n");
  clean(*build("xsmpasb", R"(assembly GoodArguments
Root: demo.Root { child += S: SubAsm<A = "y", N = 2> })"));
  const auto bad = build("xsmpasb", R"(assembly BadArguments
Root: demo.Root { child += S: SubAsm<A = "y", A = "z", N = "text"> })");
  EXPECT_TRUE(has(*bad, "Duplicated template argument name."));
  EXPECT_TRUE(has(*bad, "The value shall have the type of the parameter N."));
  EXPECT_TRUE(has(*build("xsmpasb", R"(assembly HugeArgument
Root: demo.Root { child += S: SubAsm<N = 99999999999> })"),
                  "Conversion overflow for type Int32."));
  EXPECT_TRUE(has(*build("xsmpasb",
                         "assembly <N: int32 = 99999999999> HugeParameter\n"
                         "R{N}: demo.Child\n"),
                  "Conversion overflow for type Int32."));
}

TEST_F(DeploymentAssemblyValidatorTest, InstanceNamesAndImplementations) {
  add("src/shapes.xsmpcat", R"(catalogue Shapes namespace shapes {
 public abstract model Abstract {}
 public model Concrete { container Abstract[2] kids container Abstract["a"] text }
})");
  EXPECT_TRUE(
      has(*build("xsmpasb", "assembly OfAbstract\nR: shapes.Abstract\n"),
          "shall not be an instance of an abstract Component."));
  // A placeholder expands to a name, which starts with a letter.
  EXPECT_TRUE(has(*build("xsmpasb",
                         "assembly <Msg = \"hello\"> Underscored\n"
                         "_{Msg}: shapes.Concrete\n"),
                  "The expanded name '_hello' is not valid"));
  clean(*build("xsmpasb",
               "assembly <Msg = \"hello\"> Lettered\n"
               "R{Msg}: shapes.Concrete\n"));
  // A bound that is no number gives no upper limit to report.
  EXPECT_FALSE(has(*build("xsmpasb",
                          "assembly NoLimit\nR: shapes.Concrete { text += K: "
                          "shapes.Concrete }\n"),
                   "shall not contain more than"));
}

TEST_F(DeploymentAssemblyValidatorTest,
       AssemblyChecksContainerUpperBoundsAndSiblingNames) {
  auto document = build(
      "xsmpasb",
      "assembly Instances\nRoot: demo.Root { child += Same child += Same }\n");
  EXPECT_TRUE(
      has(*document, "names shall be unique at the same hierarchy level"));
  EXPECT_TRUE(
      has(*document,
          "Container 'child' shall not contain more than 1 sub-instance(s)"));
  EXPECT_FALSE(has(*document, "without an explicit implementation"));
}

TEST_F(DeploymentAssemblyValidatorTest,
       MissingAndInvalidTemplateBindingsAreDiagnosed) {
  auto document =
      build("xsmpsed",
            "schedule <Root = \"root\", Target = \"bad-name\"> Patterns\ntask "
            "Main on DemoAsm { trig {Missing}.step trig {Target}.step }\n");
  EXPECT_TRUE(
      has(*document,
          "placeholder '{Missing}' shall resolve to a Template Argument"));
  EXPECT_TRUE(
      has(*document,
          "expanded path segment 'bad-name' is not valid for SMP Level 2"));
}

TEST_F(DeploymentAssemblyValidatorTest,
       NestedAssemblyOccurrenceReferenceBoundsIncludeImportedConnections) {
  add("src/nested.xsmpasb",
      "assembly NestedAsm\nNestedRoot: demo.Root { child += Child: demo.Child "
      "interface link logger -> Child }\n");
  auto document = build("xsmpasb",
                        "assembly Connections\nRoot: demo.System { bus += Bus: "
                        "NestedAsm interface link Bus.logger -> Bus.Child }\n");
  const auto* assembly =
      cursiv::ast_cast<xsmp::Assembly>(document->parseResult.value);
  ASSERT_NE(assembly, nullptr);
  const auto* child =
      cursiv::ast_cast<xsmp::SubInstance>(assembly->model->elements[0]);
  ASSERT_NE(child, nullptr);
  ASSERT_NE(cursiv::ast_cast<xsmp::AssemblyInstance>(child->instance), nullptr);
  EXPECT_TRUE(has(*document,
                  "The Reference 'logger' of instance '/Bus' shall not be "
                  "connected more than 1 time(s)."));
}

TEST_F(DeploymentAssemblyValidatorTest,
       ReusedNestedAssemblyOccurrencesHaveIndependentReferenceCounts) {
  add("src/nested.xsmpasb",
      "assembly NestedAsm\nNestedRoot: demo.Root { child += Child: demo.Child "
      "interface link logger -> Child }\n");
  auto document = build("xsmpasb",
                        "assembly Independent\nRoot: demo.System { bus += "
                        "Left: NestedAsm bus += Right: NestedAsm }\n");
  clean(*document);
}

TEST_F(DeploymentAssemblyValidatorTest,
       RecursiveAssemblyInstancesAreRejectedWithoutUnboundedExpansion) {
  auto document = build(
      "xsmpasb",
      "assembly Recursive\nRoot: demo.System { bus += Inner: Recursive }\n");
  EXPECT_TRUE(
      has(*document, "Recursive Assembly instantiation is not allowed."));
}

TEST_F(DeploymentAssemblyValidatorTest,
       UnusedAssemblyAndScheduleParametersWarn) {
  auto assembly = build(
      "xsmpasb", R"(assembly <Used = "Left", Unused = "Right"> UsedAssembly
configure {Used}Receiver { inValue = 1i32 }
Root: demo.Root { child += {Used}Receiver: demo.Child }
)");
  EXPECT_TRUE(has(*assembly, "The Template Parameter 'Unused' is not used."));
  EXPECT_FALSE(has(*assembly, "The Template Parameter 'Used' is not used."));
  auto schedule = build("xsmpsed",
                        "schedule <Root = \"root\", Unused = 2> "
                        "UsedSchedule\ntask Main { emit \"Boot\" }\n");
  EXPECT_TRUE(has(*schedule, "The Template Parameter 'Unused' is not used."));
  EXPECT_FALSE(has(*schedule, "The Template Parameter 'Root' is not used."));
}

TEST_F(
    DeploymentAssemblyValidatorTest,
    InvalidExpandedInstanceNamesAreDiagnosedAndDeclaredMissingValuesRemainKnown) {
  auto document =
      build("xsmpasb", R"(assembly <Missing: string, Bad = "1bad"> InvalidNames
Root: demo.Root { child += {Bad}: demo.Child }
)");
  EXPECT_TRUE(
      has(*document, "The expanded name '1bad' is not valid for SMP Level 2."));
  EXPECT_TRUE(has(*document, "A template parameter needs a value."));
  EXPECT_FALSE(has(*document, "placeholder '{Missing}'"));
}

TEST_F(DeploymentAssemblyValidatorTest, AnEmptyTemplateValueIsAWrittenValue) {
  auto document = build("xsmpasb", R"(assembly <Empty = ""> EmptyValue
Root: demo.Root { child += {Empty}Child: demo.Child }
)");
  EXPECT_FALSE(has(*document, "A template parameter needs a value."));
}

TEST_F(DeploymentAssemblyValidatorTest,
       ImportedOccurrenceBindingsSurviveMultipleDescendants) {
  add("src/nested-types.xsmpcat", R"(catalogue NestedTypes namespace nested {
 public model Child { field Smp.Int32 count }
 public model Group { container Child child }
 public model Root { container Group group }
 public model System { container Root[*] bus }
})");
  add("src/imported.xsmpasb", R"(assembly <Target="default"> Imported
Root: nested.Root { group += Group: nested.Group { child += {Target}: nested.Child } }
)");
  auto document = build("xsmpasb", R"(assembly ImportedPaths
configure Left.Group.leaf { count = 1 }
configure Right.Group.other { count = 2 }
Root: nested.System {
 bus += Left: Imported<Target="leaf">
 bus += Right: Imported<Target="other">
}
)");
  clean(*document);
  for (const auto* node : xsmp::descendantsOf(*document->parseResult.value))
    if (const auto* segment =
            cursiv::ast_cast<xsmp::ConcretePathNamedSegment>(node))
      EXPECT_NE(segment->reference.get(), nullptr)
          << segment->reference.refText;
}
