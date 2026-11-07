#include <filesystem>
#include <fstream>
#include <xsmp/tas-mdk/TasMdkPythonGenerator.hpp>

#include "support/Files.hpp"
#include "support/GeneratorTestSupport.hpp"

TEST(TasMdkPythonGenerator, UserCodeIsPreserved) {
  xsmp::ToolFixture fixture;
  const auto& catalogue = fixture.parse(
      "catalogue Demo\nnamespace n { /** @uuid "
      "11111111-2222-3333-4444-555555555555 */ model M { "
      "entrypoint ept_step } }\n");
  const xsmp::TasMdkPythonGenerator generator(*fixture.language);
  generator.generate(catalogue, fixture.directory);
  const auto user = fixture.directory / "helpers/builder/n/M/user_code.py";
  std::ofstream(user) << "# preserved user code\n";
  generator.generate(catalogue, fixture.directory);
  EXPECT_EQ(xsmp::readFile(user), "# preserved user code\n");
}
