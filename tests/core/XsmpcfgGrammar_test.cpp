#include <gtest/gtest.h>

#include <string>
#include <xsmp/DeploymentSemantics.hpp>
#include <xsmp/ast.hpp>

#include "support/DeploymentWorld.hpp"

TEST(XsmpcfgGrammar,
     ConfigurationPreservesDesignatedStructureValuesAndUnsigned64) {
  xsmp::DeploymentWorld world;
  constexpr std::string_view source = R"(
configuration Nominal
include Defaults at unsafe external
/root: demo.Platform {
  value = { first = -127i8, unsafe legacy = 18446744073709551615u64, 1.5f64 }
  samples = [2: 1u8, 2u16, 3u32]
  stamp = "2025-01-01T00:00:00Z"dt
  elapsed = "PT5S"d
  state = demo.Mode.On
  child: demo.Child { x = 'A' }
}
)";
  auto result = world.parse("xsmpcfg", source);
  xsmp::expectClean(result);
  auto* configuration = cursiv::ast_cast<xsmp::Configuration>(result.value);
  ASSERT_NE(configuration, nullptr);
  ASSERT_EQ(configuration->elements.size(), 2u);
  auto* root = cursiv::ast_cast<xsmp::ComponentConfiguration>(
      configuration->elements[1]);
  ASSERT_NE(root, nullptr);
  EXPECT_EQ(root->context.refText, "demo.Platform");
  auto* field = cursiv::ast_cast<xsmp::FieldValue>(root->elements[0]);
  ASSERT_NE(field, nullptr);
  auto* structure = cursiv::ast_cast<xsmp::StructureValue>(field->value);
  ASSERT_NE(structure, nullptr);
  ASSERT_EQ(structure->elements.size(), 3u);
  auto* assigned =
      cursiv::ast_cast<xsmp::CfgStructureFieldValue>(structure->elements[1]);
  ASSERT_NE(assigned, nullptr);
  EXPECT_TRUE(assigned->field->unsafe);
  auto* number = cursiv::ast_cast<xsmp::UInt64Value>(assigned->value);
  ASSERT_NE(number, nullptr);
  EXPECT_EQ(number->value, UINT64_MAX);
}

// A field may be named like a word that starts a statement of a configuration.
TEST(XsmpcfgGrammar, IncludeAndUnsafeAreNamesWhereANameGoesOn) {
  xsmp::DeploymentWorld world;
  auto result = world.parse("xsmpcfg", R"(configuration C
include { include = 1 unsafe = 2 include.x = 3 unsafe.y = 4 v = { unsafe = 5 } }
/root: n.M {
  include Other at include
  include = 6
  unsafe legacy = 7
  unsafe /absolute = 8
  v = { unsafe unsafe = 9 }
}
)");
  xsmp::expectClean(result);
  const auto* configuration =
      cursiv::ast_cast<xsmp::Configuration>(result.value);
  ASSERT_EQ(configuration->elements.size(), 2u);
  const auto* named = cursiv::ast_cast<xsmp::ComponentConfiguration>(
      configuration->elements[0]);
  ASSERT_NE(named, nullptr);
  EXPECT_EQ(xsmp::pathText(named->name), "include");
  const auto* root = cursiv::ast_cast<xsmp::ComponentConfiguration>(
      configuration->elements[1]);
  ASSERT_NE(root, nullptr);
  ASSERT_EQ(root->elements.size(), 5u);
  EXPECT_NE(cursiv::ast_cast<xsmp::ConfigurationUsage>(root->elements[0]),
            nullptr);
  EXPECT_FALSE(
      cursiv::ast_cast<xsmp::FieldValue>(root->elements[1])->field->unsafe);
  EXPECT_TRUE(
      cursiv::ast_cast<xsmp::FieldValue>(root->elements[2])->field->unsafe);
  EXPECT_TRUE(
      cursiv::ast_cast<xsmp::FieldValue>(root->elements[3])->field->absolute);
}
