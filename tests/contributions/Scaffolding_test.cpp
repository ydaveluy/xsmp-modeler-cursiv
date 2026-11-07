#include <filesystem>
#include <string>
#include <vector>
#include <xsmp/Contributions.hpp>
#include <xsmp/ProjectCreation.hpp>
#include <xsmp/utils/FileWriter.hpp>

#include "support/GeneratorTestSupport.hpp"
#include "support/GoldenTree.hpp"

namespace {

struct ScaffoldCase {
  std::string folder, projectName, profile;
  std::vector<std::string> tools;
};

class ScaffoldGolden : public ::testing::TestWithParam<ScaffoldCase> {};

}  // namespace

// A new project, whole, from each selection.
TEST_P(ScaffoldGolden, MatchesItsTree) {
  const auto& scaffold = GetParam();
  const xsmp::PinnedGenerationStamp pinned;
  const auto shared = xsmp::createXsmpSharedServices();
  xsmp::registerLanguages(*shared);
  xsmp::registerTasMdk(*shared);
  const xsmp::ScratchDirectory directory;
  const auto project = directory.path / scaffold.projectName;
  xsmp::createProject(xsmp::registry(*shared),
                      {.projectName = scaffold.projectName,
                       .projectDir = project,
                       .selectedProfileId = scaffold.profile,
                       .selectedToolIds = scaffold.tools});
  xsmp::expectMatchesGolden(project, std::filesystem::path(XSMP_FIXTURES_DIR)
                                         / "generator/scaffolds"
                                         / scaffold.folder / "expected");
}

INSTANTIATE_TEST_SUITE_P(
    Selections, ScaffoldGolden,
    ::testing::Values(
        ScaffoldCase{"no-profile", "Plain", "", {}},
        ScaffoldCase{"esa-cdk", "Mission", "esa-cdk", {}},
        // The name of the library, and of the Python test, are its identifier.
        ScaffoldCase{"tas-mdk", "My-Proj.1", "tas-mdk", {}},
        ScaffoldCase{
            "xsmp-sdk", "project-name", "xsmp-sdk", {"python", "adoc", "smp"}}),
    [](const ::testing::TestParamInfo<ScaffoldCase>& info) {
      auto name = info.param.folder;
      std::ranges::replace(name, '-', '_');
      return name;
    });

TEST(Scaffolding, ScaffoldsPreserveUserFiles) {
  xsmp::ToolFixture fixture;
  xsmp::writeFileIfChanged(fixture.directory / "CMakeLists.txt", "user code\n");
  xsmp::scaffoldXsmpSdkProject(fixture.directory, "project-name");
  EXPECT_EQ(fixture.read("CMakeLists.txt"), "user code\n");
}
