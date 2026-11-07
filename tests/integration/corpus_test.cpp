#include <gtest/gtest.h>

#include <algorithm>
#include <cctype>
#include <cursiv/workspace/Document.hpp>
#include <filesystem>
#include <string>
#include <vector>
#include <xsmp/CoreModule.hpp>
#include <xsmp/SharedCoreServices.hpp>

#include "support/DiagnosticTestSupport.hpp"

namespace {

std::vector<std::filesystem::path> xsmpcatFiles(
    const std::filesystem::path& dir) {
  std::vector<std::filesystem::path> files;
  std::error_code ec;
  if (std::filesystem::exists(dir, ec)) {
    for (const auto& entry :
         std::filesystem::recursive_directory_iterator(dir)) {
      if (entry.is_regular_file() && entry.path().extension() == ".xsmpcat") {
        files.push_back(entry.path());
      }
    }
  }
  std::ranges::sort(files);
  return files;
}

std::vector<std::filesystem::path> corpusFiles() {
  auto all = xsmpcatFiles(XSMP_BUILTINS_DIR);
  for (auto& file : xsmpcatFiles(XSMP_FIXTURES_DIR)) {
    all.push_back(std::move(file));
  }
  return all;
}

}  // namespace

class CorpusParse : public ::testing::TestWithParam<std::filesystem::path> {};

TEST_P(CorpusParse, ParsesWithoutSyntaxErrors) {
  auto shared = xsmp::createXsmpSharedServices();
  auto services = xsmp::createXsmpcatServices(*shared);
  auto& langServices = *services;
  shared->serviceRegistry->registerServices(std::move(services));

  auto document = xsmp::parsePath(GetParam().string(), langServices,
                                  /*validation=*/false);
  EXPECT_TRUE(document->parseSucceeded())
      << "syntax error while parsing " << GetParam();
}

INSTANTIATE_TEST_SUITE_P(
    Corpus, CorpusParse, ::testing::ValuesIn(corpusFiles()),
    [](const ::testing::TestParamInfo<std::filesystem::path>& info) {
      std::string name = info.param.stem().string();
      for (char& c : name) {
        if (std::isalnum(static_cast<unsigned char>(c)) == 0) {
          c = '_';
        }
      }
      return name + "_" + std::to_string(info.index);
    });
