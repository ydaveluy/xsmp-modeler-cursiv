#include <gtest/gtest.h>

#include <string>
#include <xsmp/ast.hpp>

#include "support/DeploymentWorld.hpp"

TEST(XsmplnkGrammar, LinkBasePreservesRootParentAndIndexedPaths) {
  xsmp::DeploymentWorld world;
  constexpr std::string_view source = R"(
link Wiring for Segment
/ {
  field link child.values[2] -> other.input
  event link ../source -> ./sink
  interface link unsafe logger -> child:unsafe back
  nested { field link output -> /root.input }
}
)";
  auto result = world.parse("xsmplnk", source);
  xsmp::expectClean(result);
  auto* links = cursiv::ast_cast<xsmp::LinkBase>(result.value);
  ASSERT_NE(links, nullptr);
  EXPECT_EQ(links->assembly.refText, "Segment");
  ASSERT_EQ(links->elements.size(), 1u);
  EXPECT_TRUE(links->elements[0]->name->absolute);
  ASSERT_EQ(links->elements[0]->elements.size(), 4u);
  auto* first =
      cursiv::ast_cast<xsmp::FieldLink>(links->elements[0]->elements[0]);
  ASSERT_NE(first, nullptr);
  ASSERT_EQ(first->ownerPath->elements.size(), 2u);
  auto* index =
      cursiv::ast_cast<xsmp::PathIndex>(first->ownerPath->elements[1]);
  ASSERT_NE(index, nullptr);
  EXPECT_EQ(index->index, 2);
}

TEST(XsmplnkGrammar, UnsafeIsANameWhereANameGoesOn) {
  xsmp::DeploymentWorld world;
  xsmp::expectClean(world.parse("xsmplnk", R"(link L for A
unsafe { field link unsafe -> unsafe.input }
unsafe other { interface link unsafe logger -> child:back }
)"));
}
