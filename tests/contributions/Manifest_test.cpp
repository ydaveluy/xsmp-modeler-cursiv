#include <gtest/gtest.h>

#include <xsmp/CliWorkspace.hpp>
#include <xsmp/Contributions.hpp>
#include <xsmp/Json.hpp>
#include <xsmp/XsmpLanguageServer.hpp>
#include <xsmp/utils/FileWriter.hpp>

#include "support/Files.hpp"
#include "support/LspTestSupport.hpp"

TEST(Manifest, AcceptsTheRangesThatIncludeTheApiVersion) {
  for (const auto* range : {"*", "x", "2", "2.x", "2.0", "2.0.x", "2.0.0", "^2",
                            "^2.0.0", "~2", "~2.0.0", ">=2.0.0", ">=1.0.0"})
    EXPECT_TRUE(xsmp::supportsExtensionApiVersion(range)) << range;
}

TEST(Manifest, RefusesEverythingElse) {
  for (const auto* range :
       {"", "1", "3", "2.1", "2.0.1", "^1.0.0", "^3.0.0", "^2.0.1", "~2.1.0",
        "~2.0.1", ">=2.0.1", "<3.0.0", "2.0.0 || 3.0.0", "2.0.0-rc.1", "any"})
    EXPECT_FALSE(xsmp::supportsExtensionApiVersion(range)) << range;
}

TEST(Manifest, ASingleContributionEntryNeedsNoArray) {
  auto world = xsmp::xsmpcatWorkspace();
  xsmp::XsmpLanguageServer server(world.shared());
  const xsmp::Json entry =
      xsmp::JsonObject{{"descriptorPath", "/missing/tool.xsmptool"}};
  EXPECT_EQ(xsmp::customRequest(server, "xsmp/registerContributions", entry),
            xsmp::customRequest(server, "xsmp/registerContributions",
                                xsmp::JsonArray{entry}));
}

TEST(Manifest, APackageChoosesTheHandlerOfThisKindOfServer) {
  auto shared = xsmp::createXsmpSharedServices();
  xsmp::registerLanguages(*shared);
  xsmp::ScratchDirectory directory;
  xsmp::writeFileIfChanged(directory.path / "tool.xsmptool", "tool 'tool'\n");
  // None of these handlers exists: the failure says which one was chosen, and
  // nothing runs.
  const auto chosen = [&](xsmp::JsonObject entry) {
    entry.insert("descriptor", "tool.xsmptool");
    entry.insert("apiVersion", "^2.0.0");
    xsmp::writeFileIfChanged(
        directory.path / "package.json",
        ::lsp::json::stringify(xsmp::JsonObject{
            {"name", "package"},
            {"contributes",
             xsmp::JsonObject{{"xsmp", xsmp::JsonArray{std::move(entry)}}}}}));
    const auto report =
        xsmp::Registry{}.registerPackage(*shared, directory.path);
    return xsmp::stringAt(
        report.object().get("failures").array().front().object(),
        "handlerPath");
  };
  EXPECT_EQ(chosen({{"handler", "./any"}}), "./any");
  EXPECT_EQ(
      chosen({{"handler", "./any"}, {"nativeHandler", xsmp::JsonObject{}}}),
      "./any");
#if defined(__wasi__)
  EXPECT_EQ(chosen({{"handler", "./any"}, {"wasiHandler", "./wasi"}}),
            "./wasi");
  EXPECT_EQ(chosen({{"handler", "./any"}, {"nativeHandler", "./native"}}),
            "./any");
#else
  EXPECT_EQ(chosen({{"handler", "./any"}, {"nativeHandler", "./native"}}),
            "./native");
  EXPECT_EQ(chosen({{"handler", "./any"}, {"wasiHandler", "./wasi"}}), "./any");
  EXPECT_EQ(chosen({{"nativeHandler", "./native"}}), "./native");
#endif
}
