
#include "support/Files.hpp"
#include "support/LspTestSupport.hpp"

TEST(XsmpsedFormatter, ScheduleFormatterFixture) {
  auto world = xsmp::deploymentWorkspace();
  (void)cursiv::test::expectFormatting(
      world.language("xsmpsed"),
      {.before = xsmp::readFixture("lsp/not-formatted.xsmpsed"),
       .after = xsmp::readFixture("lsp/formatted.xsmpsed")});
}
