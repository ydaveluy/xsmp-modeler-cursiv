
#include "support/Files.hpp"
#include "support/LspTestSupport.hpp"

TEST(XsmpasbFormatter, AssemblyFormatterFixture) {
  auto world = xsmp::deploymentWorkspace();
  (void)cursiv::test::expectFormatting(
      world.language("xsmpasb"),
      {.before = xsmp::readFixture("lsp/not-formatted.xsmpasb"),
       .after = xsmp::readFixture("lsp/formatted.xsmpasb")});
}
