#pragma once

#include <gtest/gtest.h>

#include <cursiv/utils/UriUtils.hpp>
#include <filesystem>
#include <memory>
#include <span>
#include <string>
#include <xsmp/Builtins.hpp>
#include <xsmp/Contributions.hpp>
#include <xsmp/CoreModule.hpp>
#include <xsmp/LspModule.hpp>
#include <xsmp/XsmpprojectCoreModule.hpp>
#include <xsmp/tas-mdk/TasMdkContribution.hpp>

#include "support/Files.hpp"
#include "support/LspTestSupport.hpp"

namespace xsmp {

// A language server workspace with builtins, descriptors and TAS-MDK.
inline TestWorkspace generatorWorkspace() {
  auto world = TestWorkspace::withShared<XsmpSharedServices>();
  auto& shared = world.shared();
  shared.projectManager = std::make_unique<ProjectManager>(shared);
  world.registerLanguage(createXsmpcatServices(shared));
  world.registerLanguage(createXsmpprojectServices(shared));
  loadXsmpcatBuiltins(shared);
  loadBuiltinDescriptors(shared);
  registerTasMdk(shared);
  return world;
}

inline const XsmpcatCoreServices& xsmpcatServices(TestWorkspace& world) {
  return dynamic_cast<const XsmpcatCoreServices&>(world.language("xsmpcat"));
}

// Core services without a language server, and a folder for the output.
struct ToolFixture {
  std::unique_ptr<XsmpSharedCoreServices> shared = createXsmpSharedServices();
  XsmpcatCoreServices* language;
  std::filesystem::path directory = makeScratchDirectory("xsmp-tools-");

  ToolFixture() {
    registerXsmpprojectServices(*shared);
    auto services = createXsmpcatServices(*shared);
    language = services.get();
    shared->serviceRegistry->registerServices(std::move(services));
    loadXsmpcatBuiltins(*shared);
  }

  ~ToolFixture() {
    std::error_code error;
    std::filesystem::remove_all(directory, error);
  }

  const Catalogue& parse(std::string text) {
    auto document = shared->documents->createDocument("file:///tool.xsmpcat",
                                                      std::move(text));
    auto* raw = document.get();
    shared->documentBuilder->build(std::span(&raw, 1), {.validation = false});
    EXPECT_TRUE(document->parseSucceeded());
    return *cursiv::ast_cast<const Catalogue>(document->parseResult.value);
  }

  std::string read(const std::filesystem::path& relative) const {
    return readFile(directory / relative);
  }
};

}  // namespace xsmp
