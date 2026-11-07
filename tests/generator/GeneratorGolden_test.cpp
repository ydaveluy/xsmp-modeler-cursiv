#include <algorithm>
#include <cctype>
#include <filesystem>
#include <map>
#include <string>
#include <vector>
#include <xsmp/Contributions.hpp>

#include "support/GoldenTree.hpp"

namespace {

// The contributions that generate the cases of each folder. Each case of
// `all-profiles` is generated once per profile, in a folder of its name.
const std::map<std::string, std::vector<std::string>>& generatorsOfFolders() {
  static const std::map<std::string, std::vector<std::string>> folders{
      {"adoc", {"adoc"}},
      {"all-profiles", {"xsmp-sdk", "esa-cdk", "tas-mdk"}},
      {"copyright", {"xsmp-sdk", "python", "adoc", "smp"}},
      {"esa-cdk", {"esa-cdk"}},
      {"gap-pattern", {"xsmp-sdk"}},
      {"python", {"python"}},
      {"smp", {"smp"}},
      {"tas-mdk", {"tas-mdk"}},
      {"xsmp-sdk", {"xsmp-sdk"}},
  };
  return folders;
}

const std::filesystem::path& generatorFixtures() {
  static const auto path =
      std::filesystem::path(XSMP_FIXTURES_DIR) / "generator";
  return path;
}

// Each case, as `<folder>/<case>`.
std::vector<std::string> goldenCases() {
  std::vector<std::string> cases;
  for (const auto& [folder, generators] : generatorsOfFolders()) {
    if (!std::filesystem::is_directory(generatorFixtures() / folder)) continue;
    for (const auto& entry :
         std::filesystem::directory_iterator(generatorFixtures() / folder))
      if (entry.is_directory())
        cases.push_back(folder + "/" + entry.path().filename().string());
  }
  std::ranges::sort(cases);
  return cases;
}

class GeneratorGolden : public ::testing::TestWithParam<std::string> {};

}  // namespace

TEST_P(GeneratorGolden, MatchesItsTree) {
  const std::filesystem::path relative(GetParam());
  const auto folder = relative.parent_path().string();
  const auto& generators = generatorsOfFolders().at(folder);
  xsmp::expectGoldenCase(
      generatorFixtures() / relative, [&](xsmp::GoldenWorkspace& workspace,
                                          const std::filesystem::path& output) {
        auto& registry = xsmp::registry(*workspace.shared);
        const auto nodes = workspace.nodes();
        if (folder == "all-profiles") {
          for (const auto& profile : generators)
            registry.generateBatch(nodes, output / profile, {&profile, 1});
        } else {
          registry.generateBatch(nodes, output, generators);
        }
      });
}

INSTANTIATE_TEST_SUITE_P(
    Generators, GeneratorGolden, ::testing::ValuesIn(goldenCases()),
    [](const ::testing::TestParamInfo<std::string>& info) {
      auto name = info.param;
      std::ranges::replace_if(
          name,
          [](char c) { return !std::isalnum(static_cast<unsigned char>(c)); },
          '_');
      return name;
    });
