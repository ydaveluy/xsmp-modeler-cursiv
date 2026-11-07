#include <gtest/gtest.h>

#include <filesystem>
#include <stdexcept>
#include <string>
#include <string_view>
#include <xsmp/smp/SmpService.hpp>

#include "support/Files.hpp"
#include "tools/smp/SmpWorld.hpp"

#if defined(__linux__)
TEST(SmpExport, BufferedWriteFailuresAreReportedForImportAndBothExportFiles) {
  if (!std::filesystem::exists("/dev/full")) GTEST_SKIP();
  xsmp::ScratchDirectory dir;
  const auto input = dir.write(
      "input.smpcfg",
      R"(<Configuration xmlns="urn:custom/Configuration" Name="Demo"/>)");
  EXPECT_THROW(xsmp::importFile(input, "/dev/full", true), std::runtime_error);
  xsmp::SmpWorld world;
  auto document = world.parse(".xsmpcat", "catalogue Demo namespace n {}\n");
  std::filesystem::create_directory(dir.path / "smdl-gen");
  const auto output = dir.path / "smdl-gen/roundtrip.smpcat";
  std::filesystem::create_symlink("/dev/full", output);
  EXPECT_THROW(xsmp::generateDocument(*document, dir.path), std::runtime_error);
  std::filesystem::remove(output);
  std::filesystem::create_symlink("/dev/full",
                                  dir.path / "smdl-gen/roundtrip.smppkg");
  EXPECT_THROW(xsmp::generateDocument(*document, dir.path), std::runtime_error);
}
#endif

// Dropping a start index would move the values to other items.
TEST(SmpExport, AStartIndexThatTheSchemaCannotHoldIsRefused) {
  const auto write = [](std::string_view extension, std::string text,
                        std::string_view standard) {
    xsmp::SmpWorld world;
    return xsmp::exportDocument(
        *world.parse(std::string(extension), std::move(text)), standard);
  };
  const auto configuration = [](std::string_view value) {
    return "configuration C /Root { a = " + std::string(value) + " }";
  };
  for (const auto* refused : {"[2: 3, 4]", "[2: 3i32, 4.5f64]"})
    EXPECT_THROW(write(".xsmpcfg", configuration(refused), "ECSS_SMP_2025"),
                 std::runtime_error)
        << refused;
  EXPECT_THROW(write(".xsmpcfg", configuration("[2: 3i32]"), "ECSS_SMP_2020"),
               std::runtime_error);
  EXPECT_THROW(write(".xsmpasb", R"(assembly A Root: "n::M" { a = [2: 3i32] })",
                     "ECSS_SMP_2025"),
               std::runtime_error);
  EXPECT_NO_THROW(write(".xsmpasb",
                        R"(assembly A Root: "n::M" { a = [0: 3i32] })",
                        "ECSS_SMP_2025"));
}

// A Duration is written in its canonical form, whatever its spelling.
TEST(SmpExport, DurationsAreWrittenInTheirCanonicalForm) {
  xsmp::SmpProject project("ECSS_SMP_2025");
  const auto catalogue = project.add("D.xsmpcat", R"(catalogue D
namespace n {
  public interface V {
    constant Smp.Duration a = "-PT0.000000001S"
    constant Smp.Duration b = "P1D"
    constant Smp.Duration c = "-P1D"
    constant Smp.Duration d = "PT0S"
    constant Smp.Duration e = "PT60S"
    constant Smp.Duration f = 172860000000001
    constant Smp.Duration g = -9223372036854775807 - 1
  }
})");
  project.build();
  const auto xml = xsmp::exportDocument(*catalogue);
  for (const auto* value :
       {"-PT0.000000001S", "P1D", "-P1D", "PT0S", "PT1M", "P2DT1M0.000000001S",
        "-P106751DT23H47M16.854775808S"})
    EXPECT_NE(xml.find("Value=\"" + std::string(value) + "\""),
              std::string::npos)
        << value;
}

TEST(SmpExport, AnAttributeWithoutValueIsRefusedInsteadOfWrittenInvalid) {
  xsmp::SmpWorld world;
  const auto type = world.parse(
      ".xsmpcat", "catalogue C namespace n { public attribute Smp.Bool X }");
  EXPECT_THROW(xsmp::exportDocument(*type), std::runtime_error);
}
