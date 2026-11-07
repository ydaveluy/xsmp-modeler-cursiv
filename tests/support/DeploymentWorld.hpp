#pragma once

#include <gtest/gtest.h>

#include <cursiv/parser/Parser.hpp>
#include <cursiv/services/CoreServices.hpp>
#include <cursiv/services/ServiceRegistry.hpp>
#include <cursiv/services/SharedCoreServices.hpp>
#include <cursiv/workspace/Document.hpp>
#include <cursiv/workspace/DocumentBuilder.hpp>
#include <cursiv/workspace/DocumentFactory.hpp>
#include <cursiv/workspace/Documents.hpp>
#include <deque>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <xsmp/CoreModule.hpp>
#include <xsmp/SharedCoreServices.hpp>

namespace xsmp {

// The four deployment languages, without the language server.
struct DeploymentWorld {
  std::unique_ptr<XsmpSharedCoreServices> services = createXsmpSharedServices();
  XsmpSharedCoreServices& shared = *services;
  // A parse result reads its text, so each text lives as long as the world.
  std::deque<std::string> texts;

  DeploymentWorld() {
    registerXsmpasbServices(shared);
    registerXsmpcfgServices(shared);
    registerXsmplnkServices(shared);
    registerXsmpsedServices(shared);
  }

  cursiv::ParseResult parse(std::string_view extension, std::string_view text) {
    return shared.serviceRegistry
        ->getServices(std::string("file:///workspace/test.")
                      + std::string(extension))
        .parser->parse(texts.emplace_back(text));
  }

  std::shared_ptr<cursiv::Document> build(std::string_view extension,
                                          std::string text) {
    auto document = shared.documentFactory->fromString(
        std::move(text),
        std::string("file:///workspace/test.") + std::string(extension));
    shared.documents->addDocument(document);
    auto* one = document.get();
    shared.documentBuilder->build(std::span<cursiv::Document* const>(&one, 1));
    return document;
  }
};

inline void expectClean(const cursiv::ParseResult& result) {
  ASSERT_TRUE(result.accepted);
  ASSERT_NE(result.value, nullptr);
  for (const auto& diagnostic : result.diagnostics)
    ADD_FAILURE() << diagnostic.message << " at " << diagnostic.begin;
  EXPECT_TRUE(result.diagnostics.empty());
}

}  // namespace xsmp
