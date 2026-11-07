#include "support/LspTestSupport.hpp"

TEST(DeploymentFormatter, PathIndexAndCallStayCloseToTheirName) {
  auto world = xsmp::deploymentWorkspace();
  const std::string head = "assembly A\nRoot: n.M\n{\n    ";
  (void)cursiv::test::expectFormatting(
      world.language("xsmpasb"), {.before = head + "a.b [ 1 ] = 1i32\n}\n",
                                  .after = head + "a.b[1] = 1i32\n}\n"});
  (void)cursiv::test::expectFormatting(
      world.language("xsmpasb"), {.before = head + "call op ( a = 1i32 )\n}\n",
                                  .after = head + "call op(a = 1i32)\n}\n"});
}
