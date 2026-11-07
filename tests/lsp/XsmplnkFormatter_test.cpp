
#include "support/Files.hpp"
#include "support/LspTestSupport.hpp"

TEST(XsmplnkFormatter, LinkBaseFormatterFixture) {
  auto world = xsmp::deploymentWorkspace();
  (void)cursiv::test::expectFormatting(
      world.language("xsmplnk"),
      {.before = xsmp::readFixture("lsp/not-formatted.xsmplnk"),
       .after = xsmp::readFixture("lsp/formatted.xsmplnk")});
}
