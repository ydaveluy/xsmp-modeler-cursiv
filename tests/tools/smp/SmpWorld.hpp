#pragma once

#include <cursiv/services/CoreServicesFor.hpp>
#include <cursiv/services/ServiceRegistry.hpp>
#include <cursiv/services/SharedCoreServices.hpp>
#include <cursiv/workspace/DocumentBuilder.hpp>
#include <cursiv/workspace/DocumentFactory.hpp>
#include <cursiv/workspace/Documents.hpp>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <vector>
#include <xsmp/CliWorkspace.hpp>
#include <xsmp/CoreModule.hpp>
#include <xsmp/SharedCoreServices.hpp>
#include <xsmp/generated/Xsmpcat/Module.hpp>

namespace xsmp {

// The catalogue language without its services, and the deployment languages.
struct SmpWorld {
  std::unique_ptr<XsmpSharedCoreServices> services = createXsmpSharedServices();
  XsmpSharedCoreServices& shared = *services;

  SmpWorld() {
    auto catalogue = cursiv::make_default_core_services(shared, "xsmpcat");
    generated::Xsmpcat::install_generated_module(*catalogue);
    shared.serviceRegistry->registerServices(std::move(catalogue));
    registerXsmpasbServices(shared);
    registerXsmpcfgServices(shared);
    registerXsmplnkServices(shared);
    registerXsmpsedServices(shared);
  }

  std::shared_ptr<cursiv::Document> parse(std::string extension,
                                          std::string text) {
    auto doc = shared.documentFactory->fromString(
        std::move(text), "file:///workspace/roundtrip" + extension);
    shared.documents->addDocument(doc);
    auto* one = doc.get();
    shared.documentBuilder->build(std::span<cursiv::Document* const>(&one, 1),
                                  {.validation = false});
    return doc;
  }
};

// The full languages, with their builtin libraries, around one project.
struct SmpProject {
  std::unique_ptr<XsmpSharedCoreServices> services = createXsmpSharedServices();
  XsmpSharedCoreServices& shared = *services;
  std::vector<std::shared_ptr<cursiv::Document>> documents;

  explicit SmpProject(std::string_view standard) {
    registerLanguages(shared);
    auto project = shared.documentFactory->fromString(
        "project 'Smp' using '" + std::string(standard) + "' source 'src'\n",
        "file:///smp-project/xsmp.project");
    shared.documents->addDocument(project);
    documents.push_back(std::move(project));
  }

  // A document of the source folder, built with `build`.
  std::shared_ptr<cursiv::Document> add(std::string_view name,
                                        std::string text) {
    auto document = shared.documentFactory->fromString(
        std::move(text), "file:///smp-project/src/" + std::string(name));
    shared.documents->addDocument(document);
    documents.push_back(document);
    return document;
  }

  void build() {
    std::vector<cursiv::Document*> all;
    for (const auto& document : documents) all.push_back(document.get());
    shared.documentBuilder->build(all, {.validation = false});
  }
};

}  // namespace xsmp
