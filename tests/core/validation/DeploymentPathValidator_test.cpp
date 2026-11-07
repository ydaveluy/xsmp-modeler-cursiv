#include <gtest/gtest.h>

#include <xsmp/DeploymentSemantics.hpp>
#include <xsmp/ast.hpp>
#include <xsmp/utils/TreeUtils.hpp>

#include "core/validation/DeploymentSemanticsFixture.hpp"

using DeploymentPathValidatorTest = DeploymentSemantics;

TEST_F(DeploymentPathValidatorTest,
       LongAcyclicInheritanceKeepsAllMembersAndCyclesTerminate) {
  std::string source =
      "catalogue Deep namespace deep { public model M0 { field Smp.Int32 "
      "inherited }";
  for (unsigned index = 1; index <= 70; ++index)
    source += " public model M" + std::to_string(index) + " extends M"
              + std::to_string(index - 1) + " {}";
  source +=
      " public model CycleA extends CycleB {} public model CycleB extends "
      "CycleA {} }";
  const auto catalogue = add("src/deep.xsmpcat", std::move(source));
  auto document = build(
      "xsmpasb", "assembly DeepAssembly Root: deep.M70 { inherited = 1i32 }");
  clean(*document);
  const auto* assembly =
      cursiv::ast_cast<xsmp::Assembly>(document->parseResult.value);
  ASSERT_NE(assembly, nullptr);
  ASSERT_NE(assembly->model, nullptr);
  ASSERT_EQ(assembly->model->elements.size(), 1u);
  const auto* value =
      cursiv::ast_cast<xsmp::FieldValue>(assembly->model->elements[0]);
  ASSERT_NE(value, nullptr);
  const auto* target = xsmp::pathTarget(value->field);
  ASSERT_NE(target, nullptr);
  EXPECT_EQ(cursiv::ast_cast<xsmp::Field>(target)->name, "inherited");
  const xsmp::Model* cycle = nullptr;
  for (const auto* node : xsmp::descendantsOf(*catalogue->parseResult.value))
    if (const auto* model = cursiv::ast_cast<xsmp::Model>(node);
        model && model->name == "CycleA")
      cycle = model;
  ASSERT_NE(cycle, nullptr);
  std::size_t members = 0;
  EXPECT_TRUE(xsmp::visitMembers(cycle, [&](const xsmp::NamedElement&) {
    ++members;
    return true;
  }));
  EXPECT_EQ(members, 0u);
}

TEST_F(DeploymentPathValidatorTest,
       ConfigurationRejectsImplicitNestedComponentContexts) {
  auto document = build(
      "xsmpcfg",
      "configuration Nested\n/root: demo.Root { child { inValue = 1i32 } }\n");
  EXPECT_TRUE(has(*document,
                  "A safe Component Configuration inside a Component-backed "
                  "context shall declare an explicit context."));
}

TEST_F(DeploymentPathValidatorTest,
       ArrayPathIndicesAreCheckedAgainstDeclaredBounds) {
  auto document = build(
      "xsmpasb", "assembly Bounds\nRoot: demo.Root { values[5] = 0i32 }\n");
  EXPECT_TRUE(has(*document,
                  "Array index 5 is out of bounds for the array of size 2."));
}

TEST_F(DeploymentPathValidatorTest,
       AssemblyContextPathsResolveExpandedInstanceNames) {
  auto document = build("xsmpcfg",
                        "configuration Templated\n/root: DemoAsm { Child{Lane} "
                        "{ inValue = 3 } }\n");
  clean(*document);
}

TEST_F(DeploymentPathValidatorTest,
       UnsafePathAndStructureDesignatorLinksDiffer) {
  auto document = build("xsmpcfg", R"(configuration TextReferences
/root: demo.Root { unsafe tiny = 1i32 state = { unsafe count = 1i32 } }
)");
  auto checked = false;
  for (const auto* node : xsmp::descendantsOf(*document->parseResult.value)) {
    if (const auto* segment =
            cursiv::ast_cast<xsmp::ConcretePathNamedSegment>(node);
        segment
        && (segment->unsafe
            || (segment->parent && cursiv::ast_cast<xsmp::Path>(segment->parent)
                && cursiv::ast_cast<xsmp::Path>(segment->parent)->unsafe))) {
      // A known Field in an unsafe Path is bound, while an unsafe structure
      // designator has no candidates. The unsafe marker therefore does not by
      // itself determine whether the AST reference is present.
      if (segment->unsafe)
        EXPECT_EQ(segment->reference.get(), nullptr);
      else {
        ASSERT_NE(segment->reference.get(), nullptr);
        EXPECT_EQ(segment->reference.get()->name, "tiny");
      }
      checked = true;
    }
  }
  EXPECT_TRUE(checked);
  EXPECT_FALSE(has(*document, "Could not resolve"));
}

TEST_F(DeploymentPathValidatorTest,
       ConfigurationNamesOnlyReferenceConcreteAssemblyOccurrences) {
  auto typeBacked = build("xsmpcfg", R"(configuration Typed
/Root: demo.Root { child { inValue = 2i32 } include Other at child }
)");
  const auto* root =
      cursiv::ast_cast<xsmp::Configuration>(typeBacked->parseResult.value);
  ASSERT_NE(root, nullptr);
  const auto* outer =
      cursiv::ast_cast<xsmp::ComponentConfiguration>(root->elements[0]);
  ASSERT_NE(outer, nullptr);
  const auto* child =
      cursiv::ast_cast<xsmp::ComponentConfiguration>(outer->elements[0]);
  ASSERT_NE(child, nullptr);
  const auto* segment =
      cursiv::ast_cast<xsmp::ConcretePathNamedSegment>(child->name->head);
  ASSERT_NE(segment, nullptr);
  EXPECT_EQ(segment->reference.get(), nullptr);
  const auto* include =
      cursiv::ast_cast<xsmp::ConfigurationUsage>(outer->elements[1]);
  ASSERT_NE(include, nullptr);
  const auto* includeSegment =
      cursiv::ast_cast<xsmp::ConcretePathNamedSegment>(include->path->head);
  ASSERT_NE(includeSegment, nullptr);
  EXPECT_EQ(includeSegment->reference.get(), nullptr);
  // The structural resolver can follow a declaration, but the configuration
  // context stays inactive for this missing explicit child context, so the
  // contained field has no public AST reference.
  const auto* childType = cursiv::ast_cast<xsmp::Model>(
      xsmp::targetType(xsmp::pathTarget(child->name)));
  ASSERT_NE(childType, nullptr);
  EXPECT_EQ(childType->name, "Child");
  const auto* field = cursiv::ast_cast<xsmp::FieldValue>(child->elements[0]);
  ASSERT_NE(field, nullptr);
  const auto* fieldSegment =
      cursiv::ast_cast<xsmp::ConcretePathNamedSegment>(field->field->head);
  ASSERT_NE(fieldSegment, nullptr);
  EXPECT_EQ(fieldSegment->reference.get(), nullptr);
  EXPECT_TRUE(has(*typeBacked, "shall declare an explicit context"));
  auto instanceBacked = build("xsmpcfg", R"(configuration Concrete
/Root: DemoAsm { ChildOps { inValue = 3i32 } }
)");
  const auto* concrete =
      cursiv::ast_cast<xsmp::Configuration>(instanceBacked->parseResult.value);
  ASSERT_NE(concrete, nullptr);
  const auto* concreteRoot =
      cursiv::ast_cast<xsmp::ComponentConfiguration>(concrete->elements[0]);
  ASSERT_NE(concreteRoot, nullptr);
  const auto* concreteName = cursiv::ast_cast<xsmp::ConcretePathNamedSegment>(
      concreteRoot->name->head);
  ASSERT_NE(concreteName, nullptr);
  EXPECT_EQ(concreteName->reference.get(), nullptr);
  const auto* concreteChild =
      cursiv::ast_cast<xsmp::ComponentConfiguration>(concreteRoot->elements[0]);
  ASSERT_NE(concreteChild, nullptr);
  const auto* occurrence = cursiv::ast_cast<xsmp::ConcretePathNamedSegment>(
      concreteChild->name->head);
  ASSERT_NE(occurrence, nullptr);
  ASSERT_NE(cursiv::ast_cast<xsmp::ModelInstance>(occurrence->reference.get()),
            nullptr);
  EXPECT_EQ(occurrence->reference.get()->name, "Child{Lane}");
}

TEST_F(DeploymentPathValidatorTest, OptionalPathReferencesFollowTheirScope) {
  auto links = build("xsmplnk", R"(link Links for DemoAsm
/ { interface link logger -> ChildOps }
)");
  const auto* linkRoot =
      cursiv::ast_cast<xsmp::LinkBase>(links->parseResult.value);
  ASSERT_NE(linkRoot, nullptr);
  const auto* block =
      cursiv::ast_cast<xsmp::ComponentLinkBase>(linkRoot->elements[0]);
  ASSERT_NE(block, nullptr);
  const auto* interface =
      cursiv::ast_cast<xsmp::InterfaceLink>(block->elements[0]);
  ASSERT_NE(interface, nullptr);
  const auto* source = cursiv::ast_cast<xsmp::ConcretePathNamedSegment>(
      interface->sourcePath->head);
  const auto* client = cursiv::ast_cast<xsmp::ConcretePathNamedSegment>(
      interface->clientPath->head);
  ASSERT_NE(source, nullptr);
  ASSERT_NE(client, nullptr);
  // An anchored LinkBase source goes through its assembly-only scope resolver;
  // its separate validator still resolves the endpoint.
  EXPECT_EQ(source->reference.get(), nullptr);
  ASSERT_NE(client->reference.get(), nullptr);
  EXPECT_EQ(client->reference.get()->name, "Child{Lane}");

  auto assembly = build("xsmpasb", R"(assembly Direct Root: demo.Root {
    child += Child: demo.Child
    field link unsafe outValue -> unsafe Child.inValue
    interface link logger -> .
  })");
  const auto* root =
      cursiv::ast_cast<xsmp::Assembly>(assembly->parseResult.value);
  ASSERT_NE(root, nullptr);
  ASSERT_NE(root->model, nullptr);
  const auto* field =
      cursiv::ast_cast<xsmp::FieldLink>(root->model->elements[1]);
  const auto* directInterface =
      cursiv::ast_cast<xsmp::InterfaceLink>(root->model->elements[2]);
  ASSERT_NE(field, nullptr);
  ASSERT_NE(directInterface, nullptr);
  const auto* output =
      cursiv::ast_cast<xsmp::ConcretePathNamedSegment>(field->ownerPath->head);
  const auto* inputOwner =
      cursiv::ast_cast<xsmp::ConcretePathNamedSegment>(field->clientPath->head);
  const auto* reference = cursiv::ast_cast<xsmp::ConcretePathNamedSegment>(
      directInterface->sourcePath->head);
  ASSERT_NE(output, nullptr);
  ASSERT_NE(inputOwner, nullptr);
  ASSERT_NE(reference, nullptr);
  ASSERT_NE(output->reference.get(), nullptr);
  EXPECT_EQ(output->reference.get()->name, "outValue");
  ASSERT_NE(inputOwner->reference.get(), nullptr);
  EXPECT_EQ(inputOwner->reference.get()->name, "Child");
  ASSERT_NE(reference->reference.get(), nullptr);
  EXPECT_EQ(reference->reference.get()->name, "logger");
}
