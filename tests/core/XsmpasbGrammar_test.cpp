#include <gtest/gtest.h>

#include <string>
#include <xsmp/ast.hpp>

#include "support/DeploymentWorld.hpp"

TEST(XsmpasbGrammar, AssemblyPreservesTemplatesInstancesCallsAndLinks) {
  xsmp::DeploymentWorld world;
  constexpr std::string_view source = R"(
assembly <Lane: string = "Ops", Index: int32 = 2> Segment
configure child { subscribe step -> "Start" property mode = demo.Mode.On }
Root: demo.Platform {
  slots += child: Child<Index = 3> using config Nominal using link Wiring {
    configure nested { x = [1: 7i8, 8i16, 9u32] }
  }
  field link output -> child.input
  interface link logger -> child:back
  call reset(value = 1i32)
  setting = { enabled = true, values = [1.0f32, 2.0f64] }
}

)";
  auto result = world.parse("xsmpasb", source);
  xsmp::expectClean(result);
  auto* assembly = cursiv::ast_cast<xsmp::Assembly>(result.value);
  ASSERT_NE(assembly, nullptr);
  EXPECT_EQ(assembly->name, "Segment");
  ASSERT_EQ(assembly->parameters.size(), 2u);
  auto* lane = cursiv::ast_cast<xsmp::StringParameter>(assembly->parameters[0]);
  ASSERT_NE(lane, nullptr);
  EXPECT_EQ(lane->value, "Ops");
  ASSERT_NE(assembly->model, nullptr);
  EXPECT_EQ(assembly->model->implementation.refText, "demo.Platform");
  ASSERT_EQ(assembly->model->elements.size(), 5u);
  auto* sub = cursiv::ast_cast<xsmp::SubInstance>(assembly->model->elements[0]);
  ASSERT_NE(sub, nullptr);
  auto* instance = cursiv::ast_cast<xsmp::AssemblyInstance>(sub->instance);
  ASSERT_NE(instance, nullptr);
  EXPECT_EQ(instance->assembly.refText, "Child");
  EXPECT_EQ(instance->configuration.refText, "Nominal");
  EXPECT_EQ(instance->linkBase.refText, "Wiring");
}

TEST(XsmpasbGrammar, TemplatePathPatternsRemainAtomic) {
  xsmp::DeploymentWorld world;
  for (auto spelling : {"Child{Lane}Ops", "{Lane}Ops"}) {
    const auto source =
        "configuration Demo\n" + std::string(spelling) + "\n{\n}\n";
    auto result = world.parse("xsmpcfg", source);
    xsmp::expectClean(result);
    const auto* configuration =
        cursiv::ast_cast<xsmp::Configuration>(result.value);
    ASSERT_NE(configuration, nullptr);
    ASSERT_EQ(configuration->elements.size(), 1u);
    const auto* child = cursiv::ast_cast<xsmp::ComponentConfiguration>(
        configuration->elements[0]);
    ASSERT_NE(child, nullptr);
    ASSERT_NE(child->name, nullptr);
    const auto* head =
        cursiv::ast_cast<xsmp::PatternPathNamedSegment>(child->name->head);
    ASSERT_NE(head, nullptr);
    EXPECT_EQ(head->text, spelling);
  }
}

TEST(XsmpasbGrammar, TemplatePathPatternsRejectInternalWhitespace) {
  xsmp::DeploymentWorld world;
  for (auto spelling : {"Child {Lane}Ops", "{Lane} Ops"}) {
    const auto source =
        "configuration Demo\n" + std::string(spelling) + "\n{\n}\n";
    auto result = world.parse("xsmpcfg", source);
    EXPECT_FALSE(result.diagnostics.empty()) << spelling;
    EXPECT_NE(cursiv::ast_cast<xsmp::Configuration>(result.value), nullptr);
  }
  auto result = world.parse("xsmplnk", "link Demo\nChild {Lane}Ops\n{\n}\n");
  EXPECT_FALSE(result.diagnostics.empty());
  EXPECT_NE(cursiv::ast_cast<xsmp::LinkBase>(result.value), nullptr);
}

// A field may be named like a word that starts a statement of a model.
TEST(XsmpasbGrammar, StatementWordsAreNamesWhereANameGoesOn) {
  xsmp::DeploymentWorld world;
  for (const auto* word : {"event", "field", "interface", "call", "property",
                           "subscribe", "unsafe"}) {
    SCOPED_TRACE(word);
    const auto name = std::string(word);
    auto result = world.parse(
        "xsmpasb", "assembly A Root: n.M { " + name + " = 1 x." + name + " = 2 "
                       + name + ".y = 3 " + name + " += c: n.C " + "configure "
                       + name + " { " + name + " = 4 } }");
    // `configure` is not a statement of a model: only an assembly has it.
    EXPECT_FALSE(result.diagnostics.empty());
    result = world.parse(
        "xsmpasb", "assembly A configure " + name + " { " + name + " = 4 } "
                       + "Root: n.M { " + name + " = 1 x." + name + " = 2 "
                       + name + ".y = 3 " + name + " += c: n.C }");
    xsmp::expectClean(result);
  }
  // The words keep their meaning.
  auto result = world.parse("xsmpasb", R"(assembly A Root: n.M {
  subscribe step -> "Start"
  call reset(value = 1i32)
  property mode = demo.Mode.On
  field link output -> child.input
  event link source -> child.sink
  interface link logger -> child:back
  unsafe legacy = 1
  unsafe /absolute = 1
  unsafe c += d: n.C
})");
  xsmp::expectClean(result);
  const auto* root = cursiv::ast_cast<xsmp::Assembly>(result.value)->model;
  ASSERT_EQ(root->elements.size(), 9u);
  EXPECT_NE(cursiv::ast_cast<xsmp::GlobalEventHandler>(root->elements[0]),
            nullptr);
  EXPECT_NE(cursiv::ast_cast<xsmp::OperationCall>(root->elements[1]), nullptr);
  EXPECT_NE(cursiv::ast_cast<xsmp::PropertyValue>(root->elements[2]), nullptr);
  EXPECT_NE(cursiv::ast_cast<xsmp::FieldLink>(root->elements[3]), nullptr);
  EXPECT_NE(cursiv::ast_cast<xsmp::EventLink>(root->elements[4]), nullptr);
  EXPECT_NE(cursiv::ast_cast<xsmp::InterfaceLink>(root->elements[5]), nullptr);
  const auto* legacy = cursiv::ast_cast<xsmp::FieldValue>(root->elements[6]);
  ASSERT_NE(legacy, nullptr);
  EXPECT_TRUE(legacy->field->unsafe);
  const auto* absolute = cursiv::ast_cast<xsmp::FieldValue>(root->elements[7]);
  ASSERT_NE(absolute, nullptr);
  EXPECT_TRUE(absolute->field->unsafe);
  EXPECT_TRUE(absolute->field->absolute);
  const auto* sub = cursiv::ast_cast<xsmp::SubInstance>(root->elements[8]);
  ASSERT_NE(sub, nullptr);
  EXPECT_TRUE(sub->container->unsafe);
}
