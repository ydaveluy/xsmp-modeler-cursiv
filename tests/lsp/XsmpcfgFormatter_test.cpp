
#include "support/Files.hpp"
#include "support/LspTestSupport.hpp"

TEST(XsmpcfgFormatter, ConfigurationFormatterFixture) {
  auto world = xsmp::deploymentWorkspace();
  (void)cursiv::test::expectFormatting(
      world.language("xsmpcfg"),
      {.before = xsmp::readFixture("lsp/not-formatted.xsmpcfg"),
       .after = xsmp::readFixture("lsp/formatted.xsmpcfg")});
}
