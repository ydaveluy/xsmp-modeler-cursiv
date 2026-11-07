#pragma once

#include <gtest/gtest.h>

#include <algorithm>
#include <cursiv/workspace/Document.hpp>
#include <cursiv/workspace/DocumentBuilder.hpp>
#include <cursiv/workspace/DocumentFactory.hpp>
#include <cursiv/workspace/Documents.hpp>
#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <xsmp/CliWorkspace.hpp>

// A project with a catalogue and an assembly, for the deployment languages.
struct DeploymentSemantics : ::testing::Test {
  std::unique_ptr<xsmp::XsmpSharedCoreServices> shared =
      xsmp::createXsmpSharedServices();

  std::vector<std::shared_ptr<cursiv::Document>> documents;

  void SetUp() override {
    xsmp::registerLanguages(*shared);
    add("xsmp.project",
        "project 'Semantic' using 'ECSS_SMP_2025'\nsource 'src'\n");
    add("src/types.xsmpcat", R"(catalogue Types
namespace demo {
 public integer Tiny extends Smp.UInt8 in 1 ... 3
 public float Ratio extends Smp.Float32 in 0.0 ..< 2.0
 public array IntPair = Smp.Int32[2]
 public struct State { field Smp.Int32 count field Smp.Bool enabled field IntPair values }
 public event FlagEvent extends Smp.Bool
 public model Child { input field Smp.Int32 inValue output field Smp.Int32 outValue public def void reset() entrypoint step eventsink FlagEvent inbound eventsource FlagEvent outbound reference Smp.IComponent backLogger reference Smp.IService backService }
 public model Root {
   field Smp.Bool flag
   field Tiny tiny
   field Ratio ratio
   field IntPair values
   field State state
   input field Smp.Int32 inValue
   output field Smp.Int32 outValue
   public readOnly property Smp.Bool readOnlyFlag
   public property Smp.Bool writable
   public def void apply(in Smp.Int32 count)
   entrypoint step
   eventsink FlagEvent inbound
   eventsource FlagEvent outbound
   container Child child = demo.Child
   reference Smp.IComponent logger
   reference Smp.IService serviceRef
 }
 public model System { container Root[*] bus }
})");
    add("src/assembly.xsmpasb",
        "assembly <Lane = \"Ops\"> DemoAsm\nRoot: demo.Root { child += "
        "Child{Lane}: demo.Child }\n");
    add("src/other.xsmpcfg", "configuration Other\n/Other {}\n");
  }

  std::shared_ptr<cursiv::Document> add(std::string_view path,
                                        std::string text) {
    auto document = shared->documents->createDocument(
        "file:///semantic/" + std::string(path), std::move(text));
    documents.push_back(document);
    return document;
  }

  std::shared_ptr<cursiv::Document> build(std::string_view extension,
                                          std::string text) {
    auto document = add("src/target" + std::to_string(documents.size()) + "."
                            + std::string(extension),
                        std::move(text));
    std::vector<cursiv::Document*> raw;
    for (auto& item : documents) raw.push_back(item.get());
    shared->documentBuilder->build(raw);
    EXPECT_TRUE(document->parseSucceeded());
    return document;
  }

  static bool has(const cursiv::Document& document, std::string_view text) {
    return std::ranges::any_of(document.diagnostics, [&](const auto& d) {
      return d.message.find(text) != std::string::npos;
    });
  }

  static void clean(const cursiv::Document& document) {
    for (const auto& d : document.diagnostics) ADD_FAILURE() << d.message;
  }
};
