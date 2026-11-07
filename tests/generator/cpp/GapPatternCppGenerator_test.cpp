#include <filesystem>
#include <fstream>
#include <xsmp/esa-cdk/EsaCdkGenerator.hpp>

#include "support/Files.hpp"
#include "support/GeneratorTestSupport.hpp"
#include "support/GoldenTree.hpp"

namespace {

// Nothing but this subclass writes the hand header of a class.
class ForcedGapGenerator final : public xsmp::EsaCdkGenerator {
 public:
  using EsaCdkGenerator::EsaCdkGenerator;

 protected:
  bool useGenerationGapPattern(const xsmp::Type& type) const override {
    return cursiv::ast_cast<xsmp::Class>(&type)
           || EsaCdkGenerator::useGenerationGapPattern(type);
  }
};

}  // namespace

TEST(GapPatternCppGenerator, ForcedGapSplitsAClass) {
  xsmp::expectGoldenCase(
      std::filesystem::path(XSMP_FIXTURES_DIR)
          / "generator/gap-pattern-forced/class",
      [](xsmp::GoldenWorkspace& workspace,
         const std::filesystem::path& output) {
        const ForcedGapGenerator generator(workspace.language());
        for (const auto* node : workspace.nodes())
          if (const auto* catalogue = cursiv::ast_cast<xsmp::Catalogue>(node))
            generator.generate(*catalogue, output);
      });
}

TEST(GapPatternCppGenerator, WrittenMethodBodiesArePreserved) {
  xsmp::ToolFixture fixture;
  const auto& catalogue = fixture.parse(
      "catalogue Demo\nnamespace n { public class C { public def Smp.Int32 "
      "value() } }\n");
  const xsmp::EsaCdkGenerator generator(*fixture.language);
  generator.generate(catalogue, fixture.directory);
  const auto method = fixture.directory / "src/n/C.cpp";
  std::ofstream(method) << "// user implementation\n";
  generator.generate(catalogue, fixture.directory);
  EXPECT_EQ(xsmp::readFile(method), "// user implementation\n");
}
