#include <gtest/gtest.h>

#include "core/validation/DeploymentSemanticsFixture.hpp"

using XsmplnkValidatorTest = DeploymentSemantics;

TEST_F(XsmplnkValidatorTest, LinkBasesRequireComponentBlocksAndLinks) {
  auto document = build("xsmplnk", "link Empty for DemoAsm\n/ {}\n");
  EXPECT_TRUE(
      has(*document, "A Component Link Base shall contain at least one Link."));
}

TEST_F(XsmplnkValidatorTest, LinksConnectCompatibleEnds) {
  add("src/linked.xsmpcat", R"(catalogue Linked namespace linked {
 public event EvA extends Smp.Int32
 public event EvB extends Smp.Float64
 public model M {
   eventsource EvA srcA eventsink EvA sinkA eventsink EvB sinkB
   output field Smp.Int32 oi input field Smp.Int32 ii input field Smp.Float64 if64
 }
})");
  add("src/linked.xsmpasb", "assembly Linked2\nR: linked.M\n");
  clean(*build("xsmplnk", R"(link GoodLinks for Linked2
/ {
 event link srcA -> sinkA
 field link oi -> ii
}
)"));
  const auto bad = build("xsmplnk", R"(link BadLinks for Linked2
/ {
 event link srcA -> sinkB
 field link oi -> if64
}
)");
  EXPECT_TRUE(has(*bad, "shall have the same Event Type"));
  EXPECT_TRUE(has(*bad, "same or a semantically equivalent type"));
}

TEST_F(XsmplnkValidatorTest,
       LinkBaseReferenceBoundsCountSourceAndOwnerPrefixedEndpoints) {
  auto document = build("xsmplnk", R"(link Connections for DemoAsm
/ {
 interface link logger -> ChildOps:backLogger
 interface link logger -> .
 interface link ChildOps.backLogger -> .
}
)");
  EXPECT_TRUE(has(*document,
                  "The Reference 'logger' of component path '/' shall not be "
                  "connected more than 1 time(s)."));
  EXPECT_TRUE(has(*document,
                  "The Reference 'backLogger' of component path '/ChildOps' "
                  "shall not be connected more than 1 time(s)."));
}

TEST_F(XsmplnkValidatorTest,
       InterfaceLinksValidateClientAndBackReferenceCompatibility) {
  auto document = build("xsmpasb", R"(assembly Compatibility
Root: demo.Root {
 child += Child: demo.Child
 interface link serviceRef -> Child
 interface link logger -> Child:outValue
 interface link logger -> Child:backService
}
)");
  EXPECT_TRUE(has(*document,
                  "The selected source reference shall be compatible with the "
                  "Client Component."));
  EXPECT_TRUE(has(*document,
                  "The selected reference shall resolve to a Reference of the "
                  "Client Component."));
  EXPECT_TRUE(has(
      *document,
      "The selected reference shall be compatible with the Owner Component."));
}
