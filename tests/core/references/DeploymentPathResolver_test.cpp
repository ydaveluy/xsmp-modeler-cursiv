#include <gtest/gtest.h>

#include <algorithm>
#include <cursiv/workspace/Document.hpp>
#include <cursiv/workspace/DocumentBuilder.hpp>
#include <cursiv/workspace/DocumentFactory.hpp>
#include <cursiv/workspace/Documents.hpp>
#include <cursiv/workspace/TextDocument.hpp>
#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <xsmp/CliWorkspace.hpp>
#include <xsmp/DeploymentSemantics.hpp>

TEST(DeploymentPathResolver, PathsQueriedBeforeLinkingResolveAfterTheBuild) {
  auto shared = xsmp::createXsmpSharedServices();
  xsmp::registerLanguages(*shared);
  std::vector<std::shared_ptr<cursiv::Document>> documents;
  const auto add = [&](std::string name, std::string text) {
    auto document = shared->documents->createDocument(
        "file:///prelinked/" + name, std::move(text));
    documents.push_back(document);
    return document;
  };
  add("xsmp.project", "project 'Prelinked' using 'ECSS_SMP_2025' source 'src'");
  add("src/types.xsmpcat", R"(catalogue Types namespace demo {
    public model Child {}
    public model Root { field Smp.Bool flag container Child child reference Smp.IComponent logger }
  })");
  auto config = add("src/config.xsmpcfg",
                    "configuration Config /root: demo.Root { flag = true }");
  auto links =
      add("src/links.xsmplnk",
          "link Links for Assembly / { interface link logger -> Leaf }");
  add("src/assembly.xsmpasb",
      "assembly Assembly Root: demo.Root { child += Leaf: demo.Child }");
  const auto* configuration =
      cursiv::ast_cast<xsmp::Configuration>(config->parseResult.value);
  ASSERT_NE(configuration, nullptr);
  const auto* rootConfiguration =
      cursiv::ast_cast<xsmp::ComponentConfiguration>(
          configuration->elements[0]);
  ASSERT_NE(rootConfiguration, nullptr);
  const auto* field =
      cursiv::ast_cast<xsmp::FieldValue>(rootConfiguration->elements[0]);
  ASSERT_NE(field, nullptr);
  const auto* linkBase =
      cursiv::ast_cast<xsmp::LinkBase>(links->parseResult.value);
  ASSERT_NE(linkBase, nullptr);
  const auto* link =
      cursiv::ast_cast<xsmp::InterfaceLink>(linkBase->elements[0]->elements[0]);
  ASSERT_NE(link, nullptr);
  EXPECT_EQ(xsmp::pathTarget(field->field), nullptr);
  EXPECT_EQ(xsmp::pathTarget(link->clientPath), nullptr);
  std::vector<cursiv::Document*> raw;
  for (const auto& document : documents) raw.push_back(document.get());
  shared->documentBuilder->build(raw);
  const auto* target =
      cursiv::ast_cast<xsmp::Field>(xsmp::pathTarget(field->field));
  ASSERT_NE(target, nullptr);
  EXPECT_EQ(target->name, "flag");
  const auto* child =
      cursiv::ast_cast<xsmp::ModelInstance>(xsmp::pathTarget(link->clientPath));
  ASSERT_NE(child, nullptr);
  EXPECT_EQ(child->name, "Leaf");
  EXPECT_TRUE(config->diagnostics.empty());
  EXPECT_TRUE(links->diagnostics.empty());
}

namespace {

// A project with `types`, then the given documents, built with validation.
struct Paths {
  std::unique_ptr<xsmp::XsmpSharedCoreServices> shared =
      xsmp::createXsmpSharedServices();
  std::vector<std::shared_ptr<cursiv::Document>> documents;

  explicit Paths(std::string types) {
    xsmp::registerLanguages(*shared);
    add("xsmp.project", "project 'P' using 'ECSS_SMP_2025' source 'src'");
    add("src/types.xsmpcat", std::move(types));
  }

  std::shared_ptr<cursiv::Document> add(const std::string& path,
                                        std::string text) {
    auto document = shared->documents->createDocument("file:///paths/" + path,
                                                      std::move(text));
    documents.push_back(document);
    return document;
  }

  void build() {
    std::vector<cursiv::Document*> raw;
    for (const auto& document : documents) raw.push_back(document.get());
    shared->documentBuilder->build(raw);
  }

  template <class T>
  static const T* last(const cursiv::Document& document) {
    const T* found = nullptr;
    for (const auto* node : document.parseResult.nodes())
      if (const auto* candidate = cursiv::ast_cast<T>(node)) found = candidate;
    return found;
  }

  static void clean(const cursiv::Document& document) {
    for (const auto& diagnostic : document.diagnostics)
      ADD_FAILURE() << document.uri << ": " << diagnostic.message;
  }

  static bool has(const cursiv::Document& document, std::string_view text) {
    return std::ranges::any_of(document.diagnostics, [&](const auto& d) {
      return d.message.find(text) != std::string::npos;
    });
  }
};

constexpr std::string_view kBattery = R"(catalogue T namespace n {
  public model Battery { public field Smp.Float64 power output field Smp.Float64 out }
  public model Root { container Battery[*] power public field Smp.Float64 root container Sub[*] subs output field Smp.Float64 Root }
  public model Sub { container Battery[*] cells public field Smp.Float64 v }
})";

}  // namespace

// A path written in a block is appended to the path of that block: its head
// names a member before it names the instance itself.
TEST(DeploymentPathResolver, RelativeHeadReadsAMemberBeforeTheInstanceName) {
  Paths paths{std::string(kBattery)};
  const auto assembly = paths.add("src/a.xsmpasb", R"(assembly A
root: n.Root {
  root = 1.0
  power += power: n.Battery {
    power = 2.0
  }
})");
  const auto config = paths.add("src/k.xsmpcfg", R"(configuration K
/root: A {
  root = 3.0
  power {
    power = 4.0
  }
})");
  paths.build();
  Paths::clean(*assembly);
  Paths::clean(*config);
  const auto* value = Paths::last<xsmp::FieldValue>(*assembly);
  ASSERT_NE(value, nullptr);
  const auto* field =
      cursiv::ast_cast<xsmp::Field>(xsmp::pathTarget(value->field));
  ASSERT_NE(field, nullptr);
  EXPECT_EQ(field->name, "power");
  value = Paths::last<xsmp::FieldValue>(*config);
  ASSERT_NE(value, nullptr);
  EXPECT_NE(cursiv::ast_cast<xsmp::Field>(xsmp::pathTarget(value->field)),
            nullptr);
}

// The head still names the instance where no member fits, and an absolute path
// starts at the root whatever its members are called.
TEST(DeploymentPathResolver, HeadNamesTheInstanceWhenNoMemberFits) {
  Paths paths{std::string(kBattery)};
  paths.add("src/a.xsmpasb", R"(assembly A
root: n.Root {
  power += cell: n.Battery
  power += other: n.Battery
})");
  const auto links = paths.add("src/l.xsmplnk", R"(link L for A
root {
  field link root.Root -> root.cell.out
}
/root {
  field link cell.out -> other.out
}
)");
  paths.build();
  const auto* relative = cursiv::ast_cast<xsmp::ComponentLinkBase>(
      cursiv::ast_cast<xsmp::LinkBase>(links->parseResult.value)->elements[0]);
  ASSERT_NE(relative, nullptr);
  // `root` is also a field of the model, which cannot hold links.
  EXPECT_NE(
      cursiv::ast_cast<xsmp::ModelInstance>(xsmp::pathTarget(relative->name)),
      nullptr);
  const auto* absolute = cursiv::ast_cast<xsmp::ComponentLinkBase>(
      cursiv::ast_cast<xsmp::LinkBase>(links->parseResult.value)->elements[1]);
  ASSERT_NE(absolute, nullptr);
  EXPECT_NE(
      cursiv::ast_cast<xsmp::ModelInstance>(xsmp::pathTarget(absolute->name)),
      nullptr);
}

// Assembly.xsd: the configuration of an Assembly Instance addresses a component
// "via a ComponentPath relative to the AssemblyInstance node".
TEST(DeploymentPathResolver,
     AssemblyInstanceConfigurationIsReadInThatInstance) {
  Paths paths{std::string(kBattery)};
  paths.add("src/sub.xsmpasb", R"(assembly <Idx: int32> SubAsm
top: n.Sub {
  cells += k{Idx}: n.Battery
})");
  const auto assembly = paths.add("src/a.xsmpasb", R"(assembly A
root: n.Root {
  subs += s: SubAsm<Idx = 7> {
    configure k7 { power = 3.0 }
    configure s.k7 { power = 4.0 }
  }
})");
  paths.build();
  Paths::clean(*assembly);
  const auto* instance = Paths::last<xsmp::AssemblyInstance>(*assembly);
  ASSERT_NE(instance, nullptr);
  ASSERT_EQ(instance->elements.size(), 2u);
  for (const auto* configuration : instance->elements) {
    EXPECT_EQ(xsmp::pathContext(configuration->name), instance);
    const auto* target = cursiv::ast_cast<xsmp::ModelInstance>(
        xsmp::pathTarget(configuration->name));
    ASSERT_NE(target, nullptr);
    EXPECT_EQ(target->name, "k{Idx}");
  }
}

// A path crosses types that no reference of its document names: an edit of one
// of them still reaches it.
TEST(DeploymentPathResolver, EditingATypeTheChainCrossesRevalidatesThePath) {
  Paths paths{"catalogue T namespace n { public model M { field o.S s } }"};
  const std::string withBase =
      "catalogue O namespace o { public class S extends p.B {} }";
  const auto middle = paths.add("src/o.xsmpcat", withBase);
  paths.add("src/p.xsmpcat",
            "catalogue P namespace p { public class B { field Smp.Int32 v } }");
  const auto config =
      paths.add("src/k.xsmpcfg", "configuration K /m: n.M { s.v = 1i32 }");
  paths.build();
  EXPECT_FALSE(Paths::has(*config, "shall resolve"));
  const auto edit = [&](std::string text) {
    paths.shared->documentFactory->attachTextDocument(
        *middle, cursiv::TextDocument::create(
                     middle->uri, "xsmpcat",
                     middle->textDocument().version() + 1, std::move(text)));
    const std::string_view changed[] = {middle->uri};
    paths.shared->documentBuilder->update(changed, {});
  };
  edit("catalogue O namespace o { public class S {} }");
  EXPECT_TRUE(Paths::has(*config, "shall resolve"));
  edit(withBase);
  EXPECT_FALSE(Paths::has(*config, "shall resolve"));
}

// Each block is read once per level, not twice: this depth took minutes.
TEST(DeploymentPathResolver, DeepBlocksResolveInLinearTime) {
  constexpr auto depth = 40;
  std::string types =
      "catalogue T namespace n { public model L0 { output field Smp.Int32 out "
      "} ";
  for (auto level = 1; level <= depth; ++level)
    types += "public model L" + std::to_string(level) + " { container L"
             + std::to_string(level - 1) + "[*] c } ";
  Paths paths{types + "}"};
  std::string instances, blocks, closing;
  for (auto level = depth; level > 0; --level) {
    instances += "c += i" + std::to_string(level) + ": n.L"
                 + std::to_string(level - 1) + " { ";
    blocks += "i" + std::to_string(level) + " { ";
    closing += "} ";
  }
  paths.add("src/a.xsmpasb", "assembly A root: n.L" + std::to_string(depth)
                                 + " { " + instances + closing + "}");
  const auto links = paths.add("src/l.xsmplnk", "link L for A /root { " + blocks
                                                    + "field link out -> out "
                                                    + closing + "}");
  paths.build();
  const auto* link = Paths::last<xsmp::FieldLink>(*links);
  ASSERT_NE(link, nullptr);
  EXPECT_NE(cursiv::ast_cast<xsmp::Field>(xsmp::pathTarget(link->ownerPath)),
            nullptr);
}
