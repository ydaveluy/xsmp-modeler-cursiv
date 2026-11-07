#include <gtest/gtest.h>

#include <cursiv/workspace/Documents.hpp>
#include <xsmp/CoreModule.hpp>
#include <xsmp/Json.hpp>
#include <xsmp/XsmpLanguageServer.hpp>
#include <xsmp/XsmpTextDocumentContentProvider.hpp>
#include <xsmp/ast.hpp>
#include <xsmp/validation/IssueCodes.hpp>
#include <xsmp/workspace/SourceUri.hpp>

#include "support/LspTestSupport.hpp"

TEST(SourceUri, SmpMirrorsRoundTripAllFiveKindsAndAreReadOnly) {
  for (const auto extension :
       {"smpcat", "smpasb", "smpcfg", "smplnk", "smpsed"}) {
    const auto source =
        "file:///workspace/My%20Project/smdl/Test." + std::string(extension);
    const auto mirror = xsmp::smpMirrorUri(source);
    EXPECT_TRUE(xsmp::isSmpMirrorUri(mirror));
    EXPECT_TRUE(xsmp::isReadOnlyDocumentUri(mirror));
    EXPECT_EQ(xsmp::sourceDocumentUri(mirror), source);
  }
  EXPECT_TRUE(xsmp::isReadOnlyDocumentUri("xsmp:/ecss.smp.xsmpcat"));
  EXPECT_FALSE(xsmp::isReadOnlyDocumentUri("file:///workspace/Test.xsmpcat"));
  EXPECT_TRUE(xsmp::smpMirrorUri("file:///workspace/Test.xsmpcat").empty());
}

// A mirror keeps the name of its XML file, whose extension gives its language.
// The mirror URI of an earlier version, which ended with the native extension,
// still leads to its XML file, but stands for no document.
TEST(SourceUri, AMirrorKeepsTheNameOfItsXmlFile) {
  const auto mirror = xsmp::smpMirrorUri("file:///workspace/model.smpcfg");
  EXPECT_EQ(mirror, "xsmp-smp:/workspace/model.smpcfg");
  EXPECT_EQ(xsmp::smpMirrorNativeExtension(mirror), ".xsmpcfg");
  EXPECT_EQ(xsmp::nativeFileName(mirror), "model.xsmpcfg");
  EXPECT_EQ(xsmp::nativeFileName("file:///workspace/other.xsmpcat"),
            "other.xsmpcat");
  const std::string legacy = "xsmp-smp:/workspace/model.xsmpcfg";
  EXPECT_EQ(xsmp::sourceDocumentUri(legacy), "file:///workspace/model.smpcfg");
  EXPECT_TRUE(xsmp::smpMirrorNativeExtension(legacy).empty());

  const auto shared = xsmp::createXsmpSharedServices();
  xsmp::registerXsmpServices(*shared);
  const auto& registry = *shared->serviceRegistry;
  EXPECT_EQ(registry.getServices(mirror, "xml").languageMetaData.languageId,
            "xsmpcfg");
  EXPECT_FALSE(registry.hasServices(legacy));
  EXPECT_FALSE(registry.hasServices("file:///workspace/model.smpcfg"));
  EXPECT_THROW((void)registry.getServices(legacy),
               cursiv::ServiceRegistryError);
}

TEST(SourceUri, SmpMirrorExtensionsPreserveQueryAndFragment) {
  for (const auto suffix :
       {"?revision%3D1", "#section", "?revision%3D1#section"}) {
    const auto source = std::string("file:///workspace/model.smpcat") + suffix;
    const auto mirror = xsmp::smpMirrorUri(source);
    EXPECT_EQ(mirror, std::string("xsmp-smp:/workspace/model.smpcat") + suffix);
    EXPECT_EQ(xsmp::sourceDocumentUri(mirror), source);
  }
}

TEST(SourceUri,
     RelativeSourcePathsKeepLiteralUriPunctuationAndPortableSeparators) {
  EXPECT_EQ(xsmp::resolveSourceUri("file:///workspace/project",
                                   "src #?%2F/data.xsmpcat"),
            "file:///workspace/project/src%20%23%3F%252F/data.xsmpcat");
  EXPECT_EQ(xsmp::resolveSourceUri("file:///workspace/project",
                                   R"(src\sub\..\data.xsmpcat)"),
            "file:///workspace/project/src/data.xsmpcat");
}

#include <cursiv/services/ServiceRegistry.hpp>
#include <cursiv/workspace/DocumentBuilder.hpp>
#include <cursiv/workspace/Documents.hpp>
#include <cursiv/workspace/IndexManager.hpp>
#include <xsmp/SharedCoreServices.hpp>

TEST(
    SourceUri,
    HeldNativeDocumentsSuppressShadowedMirrorExportsForCatalogueAndDeployment) {
  auto shared = xsmp::createXsmpSharedServices();
  shared->serviceRegistry->registerServices(
      xsmp::createXsmpcatServices(*shared));
  xsmp::registerXsmpasbServices(*shared);
  xsmp::registerXsmpcfgServices(*shared);
  xsmp::registerXsmplnkServices(*shared);
  xsmp::registerXsmpsedServices(*shared);
  auto native = shared->documents->createDocument(
      "file:///sidecar/model.xsmpcat",
      "catalogue Native\nnamespace n { struct A {} }\n");
  auto mirror = shared->documents->createDocument(
      "xsmp-smp:/sidecar/model.smpcat",
      "catalogue Mirror\nnamespace n { struct A {} }\n");
  auto config = shared->documents->createDocument(
      "file:///sidecar/nominal.xsmpcfg", "configuration NativeConfiguration\n");
  auto configMirror =
      shared->documents->createDocument("xsmp-smp:/sidecar/nominal.smpcfg",
                                        "configuration MirrorConfiguration\n");
  cursiv::Document* documents[] = {native.get(), mirror.get(), config.get(),
                                   configMirror.get()};
  shared->documentBuilder->build(documents, {.validation = false});
  EXPECT_TRUE(xsmp::hasNativeSidecar(mirror->uri, *shared));
  EXPECT_TRUE(xsmp::hasNativeSidecar(configMirror->uri, *shared));
  EXPECT_FALSE(
      xsmp::hasNativeSidecar("xsmp-smp:/sidecar/memory-only.smpcat", *shared));
  std::size_t mirrorSymbols = 0;
  (void)shared->indexManager->visitElements(
      nullptr, {}, [&](const cursiv::AstNodeDescription& symbol) {
        if (symbol.documentId == mirror->id
            || symbol.documentId == configMirror->id)
          ++mirrorSymbols;
        return true;
      });
  EXPECT_EQ(mirrorSymbols, 0u);
}

#include <cursiv/utils/UriUtils.hpp>
#include <filesystem>
#include <fstream>

TEST(SourceUri,
     PhysicalNativeSidecarShadowsMirrorWithoutAnyXmlSourceOrHeldDocument) {
  auto shared = xsmp::createXsmpSharedServices();
  const auto directory =
      std::filesystem::temp_directory_path()
      / ("xsmp-sidecar-core-"
         + std::to_string(reinterpret_cast<std::uintptr_t>(shared.get())));
  std::filesystem::create_directories(directory);
  const auto native = directory / "model.xsmpcat";
  {
    std::ofstream file(native);
    file << "catalogue Native\n";
  }
  auto source = native;
  source.replace_extension(".smpcat");
  const auto mirror =
      xsmp::smpMirrorUri(cursiv::path_to_file_uri(source.string()));
  EXPECT_FALSE(std::filesystem::exists(source));
  EXPECT_TRUE(xsmp::hasNativeSidecar(mirror, *shared));
  std::error_code error;
  std::filesystem::remove_all(directory, error);
}

TEST(SourceUri, VirtualUrisHaveOneSpelling) {
  for (const auto& [spelled, canonical] :
       std::initializer_list<std::pair<std::string_view, std::string_view>>{
           {"xsmp:///probe@ECSS_SMP_2025.xsmpcat",
            "xsmp:/probe%40ECSS_SMP_2025.xsmpcat"},
           {"xsmp:/probe%40ECSS_SMP_2025.xsmpcat",
            "xsmp:/probe%40ECSS_SMP_2025.xsmpcat"},
           {"xsmp:///virtual/a%23b%3Fc%25d%2540.xsmpcat",
            "xsmp:/virtual/a%23b%3Fc%25d%2540.xsmpcat"},
           {"xsmp-smp:///model%23%3F%25.smpcat",
            "xsmp-smp:/model%23%3F%25.smpcat"},
           // A Windows path: the drive letter goes to lower case, and the
           // separators stay `/` on every system.
           {"xsmp-smp:///C:/work/model.smpcat",
            "xsmp-smp:/c%3A/work/model.smpcat"},
           {"xsmp-smp://localhost/c%3A/work/model.smpcat",
            "xsmp-smp:/c%3A/work/model.smpcat"},
           {"xsmp-smp://server/work/model.smpcat",
            "xsmp-smp://server/work/model.smpcat"}})
    EXPECT_EQ(xsmp::canonicalVirtualUri(spelled), canonical);
  EXPECT_EQ(xsmp::smpMirrorUri("file:///C:/work/model.smpcat"),
            "xsmp-smp:/c%3A/work/model.smpcat");
}
