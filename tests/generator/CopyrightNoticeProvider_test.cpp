#include <cstdlib>
#include <regex>
#include <xsmp/xsmp-sdk/XsmpSdkGenerator.hpp>

#include "support/GeneratorTestSupport.hpp"

// The goldens pin the time, so this is the one test of the present time:
// `${date}` reads as `2026-01-31` and `${time}` as `13:45:09`.
TEST(CopyrightNoticeProvider, WithoutAPinnedTimeTheBannerNamesThePresent) {
  if (std::getenv("SOURCE_DATE_EPOCH")) GTEST_SKIP();
  xsmp::ToolFixture fixture;
  const auto& catalogue =
      fixture.parse("// ${date} ${time}\ncatalogue Stamp\nnamespace demo {}\n");
  xsmp::XsmpSdkGenerator(*fixture.language)
      .generate(catalogue, fixture.directory);
  const auto header = fixture.read("src-gen/Stamp.h");
  EXPECT_TRUE(std::regex_search(
      header, std::regex(R"(^// \d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}\n)")))
      << header;
}
