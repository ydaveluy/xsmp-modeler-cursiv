#include <xsmp/Builtins.hpp>
#include <xsmp/Json.hpp>

#include "support/LspTestSupport.hpp"

namespace {

const char* catalogue = R"XSMP(catalogue Demo

namespace demo
{
    public enum Mode
    {
        Nominal = 0,
        Safe = 1
    }

    public interface ILogger
    {
    }

    public struct State
    {
        field Smp.Bool enabled
        field Smp.Int32 count
    }

    public array BoolPair = Smp.Bool[2]

    public model Child
    {
        input field Smp.Int32 inValue
        output field Smp.Int32 outValue
        public def void reset()
        reference demo.ILogger backLogger
    }

    public model Root
    {
        field State state
        field Smp.Bool flag
        field BoolPair flags
        output field Smp.Int32 outValue
        input field Smp.Int32 inValue
        readWrite property demo.Mode mode
        public def void apply(in demo.Mode requestedMode)
        entrypoint step
        container Child child = demo.Child
        reference demo.ILogger logger
    }
}
)XSMP";

xsmp::TestWorkspace workspace() {
  auto world = xsmp::TestWorkspace::withShared<xsmp::XsmpSharedServices>();
  auto& shared = world.shared();
  shared.projectManager = std::make_unique<xsmp::ProjectManager>(shared);
  world.registerLanguage(xsmp::createXsmpcatServices(shared));
  world.registerLanguage(xsmp::createXsmpprojectServices(shared));
  xsmp::registerXsmpasbServices(shared);
  xsmp::registerXsmpcfgServices(shared);
  xsmp::registerXsmplnkServices(shared);
  xsmp::registerXsmpsedServices(shared);
  xsmp::loadXsmpcatBuiltins(shared);
  (void)xsmp::parse(
      world, "xsmpproject",
      "project 'Completion' using 'ECSS_SMP_2025'\nsource 'smdl'\n",
      "file:///completion/xsmp.project");
  (void)xsmp::parse(world, "xsmpcat", catalogue,
                    "file:///completion/smdl/Demo.xsmpcat");
  (void)xsmp::parse(
      world, "xsmpasb",
      "assembly DemoAsm\nRoot: demo.Root\n{ child += Child: demo.Child }\n",
      "file:///completion/smdl/DemoAsm.xsmpasb");
  return world;
}

::lsp::CompletionList completion(xsmp::TestWorkspace& world,
                                 std::string_view id, std::string text) {
  ::lsp::CompletionList list;
  (void)cursiv::test::expectCompletion(
      world.language(id),
      cursiv::test::ExpectedCompletionCallback{
          .text = std::move(text),
          .assertion =
              [&](const ::lsp::CompletionList& answer) { list = answer; },
          .parseOptions = xsmp::documentAt("file:///completion/smdl/test."
                                           + std::string(id)),
          .disposeAfterCheck = true});
  return list;
}

bool has(const ::lsp::CompletionList& list, std::string_view label) {
  return std::ranges::any_of(
      list.items, [&](const auto& item) { return item.label == label; });
}

std::string insertion(const ::lsp::CompletionList& list,
                      std::string_view label) {
  // An enriched snippet is preferred to a plain reference with the same label
  // (notably schedule tasks).
  for (const auto& item : list.items)
    if (item.label == label
        && item.insertTextFormat == ::lsp::InsertTextFormat::Snippet)
      return xsmp::completionText(item);
  for (auto& item : list.items)
    if (item.label == label) return xsmp::completionText(item);
  return {};
}

}  // namespace

TEST(DeploymentCompletionProvider, ValuesDoNotDependOnTheLayout) {
  auto world = workspace();
  for (const auto* text :
       {"configuration Demo\n/Root: demo.Root\n{\n flag =\n <|>\n}\n",
        "configuration Demo\n/Root: demo.Root { flag = <|> }\n",
        "configuration Demo\n/Root: demo.Root\n{\n flag = // why\n <|>\n}\n"}) {
    SCOPED_TRACE(text);
    const auto values = completion(world, "xsmpcfg", text);
    EXPECT_TRUE(has(values, "false"));
    EXPECT_TRUE(has(values, "true"));
    EXPECT_FALSE(has(values, "Component Configuration"));
  }
}

TEST(DeploymentCompletionProvider,
     ReferencesAreOfferedWhereverTheGrammarReadsThem) {
  auto world = workspace();
  const auto members =
      completion(world, "xsmpcfg",
                 "configuration Demo\n/Root: demo.Root\n{\n state.<|>\n}\n");
  EXPECT_TRUE(has(members, "enabled"));
  EXPECT_TRUE(has(members, "count"));
  EXPECT_FALSE(has(members, "flag"));
  EXPECT_TRUE(
      has(completion(world, "xsmpcfg", "configuration Demo\n/Root: <|>"),
          "demo.Root"));
  EXPECT_TRUE(
      has(completion(world, "xsmpsed", "schedule Demo\ntask Main on <|>"),
          "demo.Root"));
  const auto output = completion(
      world, "xsmplnk", "link Demo for DemoAsm\n/\n{\n field link <|>\n}\n");
  EXPECT_TRUE(has(output, "outValue"));
  EXPECT_FALSE(has(output, "inValue"));
  const auto input = completion(
      world, "xsmplnk",
      "link Demo for DemoAsm\n/\n{\n field link outValue -> <|>\n}\n");
  EXPECT_TRUE(has(input, "inValue"));
  EXPECT_FALSE(has(input, "outValue"));
}

TEST(DeploymentCompletionProvider, ValueOfAPathWithSeveralSegments) {
  auto world = workspace();
  const auto values = completion(
      world, "xsmpcfg",
      "configuration Demo\n/Root: demo.Root\n{\n state.enabled = <|>\n}\n");
  EXPECT_TRUE(has(values, "false"));
  EXPECT_TRUE(has(values, "true"));
  EXPECT_FALSE(has(values, "demo.Mode.Safe"));
}

TEST(DeploymentCompletionProvider, RootSnippetsReplaceBareKeywords) {
  auto world = workspace();
  for (auto [id, label, keyword] : std::array<
           std::tuple<std::string_view, std::string_view, std::string_view>, 4>{
           {{"xsmpasb", "Assembly", "assembly"},
            {"xsmpcfg", "Configuration", "configuration"},
            {"xsmplnk", "Link Base", "link"},
            {"xsmpsed", "Schedule", "schedule"}}}) {
    const auto list = completion(world, id, "<|>");
    EXPECT_TRUE(has(list, label)) << id;
    EXPECT_FALSE(has(list, keyword)) << id;
  }
}

TEST(DeploymentCompletionProvider,
     CommentsContainingDeclarationKeywordsDoNotCreateRoots) {
  auto world = workspace();
  for (auto [id, label, keyword] : std::array<
           std::tuple<std::string_view, std::string_view, std::string_view>, 4>{
           {{"xsmpasb", "Assembly", "assembly"},
            {"xsmpcfg", "Configuration", "configuration"},
            {"xsmplnk", "Link Base", "link"},
            {"xsmpsed", "Schedule", "schedule"}}}) {
    const auto list = completion(
        world, id,
        "// An example of a " + std::string(keyword) + " document.\n<|>");
    EXPECT_TRUE(has(list, label)) << id;
    EXPECT_EQ(insertion(list, label),
              std::string(keyword) + " ${1:Name}"
                  + (id == "xsmplnk" ? " for ${2|DemoAsm|}" : "") + "\n$0")
        << id;
  }
}

TEST(DeploymentCompletionProvider,
     LinkBaseDefinitionIncludesAvailableAssembly) {
  auto world = workspace();
  const auto list = completion(world, "xsmplnk", "<|>");
  EXPECT_EQ(insertion(list, "Link Base"),
            "link ${1:Name} for ${2|DemoAsm|}\n$0");
  auto empty = xsmp::TestWorkspace::withShared<xsmp::XsmpSharedServices>();
  empty.shared().projectManager =
      std::make_unique<xsmp::ProjectManager>(empty.shared());
  empty.registerLanguage(xsmp::createXsmpcatServices(empty.shared()));
  xsmp::registerXsmpasbServices(empty.shared());
  xsmp::registerXsmpcfgServices(empty.shared());
  xsmp::registerXsmplnkServices(empty.shared());
  xsmp::registerXsmpsedServices(empty.shared());
  EXPECT_EQ(insertion(completion(empty, "xsmplnk", "<|>"), "Link Base"),
            "link ${1:Name}\n$0");
}

TEST(DeploymentCompletionProvider, SnippetDetailsAndPriorities) {
  auto world = workspace();
  const auto expect = [](const auto& list, std::string_view label,
                         std::string_view detail) {
    const auto item = std::ranges::find_if(
        list.items, [&](const auto& value) { return value.label == label; });
    ASSERT_NE(item, list.items.end()) << label;
    EXPECT_EQ(item->detail, detail);
    EXPECT_EQ(item->kind, ::lsp::CompletionItemKind::Snippet);
    EXPECT_EQ(item->sortText, "3000_" + std::string(label));
  };
  expect(completion(world, "xsmplnk", "link Demo\n<|>"),
         "Root Component Link Base", "Root Component Link Base");
  expect(completion(world, "xsmplnk", "link Demo for DemoAsm\n/\n{\n<|>\n}"),
         "Component Link Base", "Nested Component Link Base");
  const auto events = completion(world, "xsmpsed", "schedule Demo\n<|>");
  for (const auto [label, detail] :
       std::array<std::pair<std::string_view, std::string_view>, 4>{
           {{"Event Epoch", "Epoch Event"},
            {"Event Mission", "Mission Event"},
            {"Event Simulation", "Simulation Event"},
            {"Event Zulu", "Zulu Event"}}})
    expect(events, label, detail);
}

TEST(DeploymentCompletionProvider,
     ContextualValuesRetainSnippetTextAndMetadata) {
  auto world = workspace();
  const auto fields = completion(
      world, "xsmpcfg", "configuration Demo\n/root: demo.Root\n{\n<|>\n}");
  const auto flag = std::ranges::find_if(
      fields.items, [](const auto& value) { return value.label == "flag"; });
  ASSERT_NE(flag, fields.items.end());
  EXPECT_EQ(flag->kind, ::lsp::CompletionItemKind::Value);
  EXPECT_EQ(flag->insertTextFormat, ::lsp::InsertTextFormat::Snippet);
  EXPECT_EQ(flag->detail, "Configurable field of Root.");
  EXPECT_EQ(flag->sortText, "2000_flag");
  EXPECT_EQ(flag->filterText, "flag");
  EXPECT_EQ(xsmp::completionText(*flag), "flag = ${1:false}");
  const auto values =
      completion(world, "xsmpcfg",
                 "configuration Demo\n/root: demo.Root\n{\nflag = <|>\n}");
  const auto boolean = std::ranges::find_if(
      values.items, [](const auto& value) { return value.label == "false"; });
  ASSERT_NE(boolean, values.items.end());
  EXPECT_EQ(boolean->kind, ::lsp::CompletionItemKind::Value);
  EXPECT_FALSE(boolean->insertTextFormat);
  EXPECT_EQ(boolean->detail, "Boolean value for Smp.Bool.");
  EXPECT_EQ(boolean->sortText, "2000_false");
}

TEST(DeploymentCompletionProvider,
     ScheduleStringDefaultsDistinguishMissingAndEmpty) {
  auto world = workspace();
  for (const auto& [declaration, expected] :
       std::array<std::pair<std::string, std::string>, 2>{
           {{"Root: string", "${1:\"value\"}"},
            {"Root: string = \"\"", "${1:}"}}}) {
    const auto list =
        completion(world, "xsmpsed",
                   "schedule <" + declaration +
                       "> Demo\ntask Main on demo.Root\n{\n execute "
                       "<|>\n}\ntask Worker on demo.Child {}\n");
    EXPECT_EQ(insertion(list, "Worker"),
              "Worker<Root = " + expected + "> at ${2:path}");
  }
}

TEST(DeploymentCompletionProvider,
     UnresolvedPropertyTypesUseEditableValueFallback) {
  auto world = workspace();
  (void)xsmp::parse(
      world, "xsmpcat",
      "catalogue Extra namespace demo { public model MissingPropertyType { "
      "readWrite property Missing missing } }",
      "file:///completion/smdl/Extra.xsmpcat");
  const auto list = completion(
      world, "xsmpsed",
      "schedule Demo\ntask Main on demo.MissingPropertyType\n{\n<|>\n}");
  EXPECT_EQ(insertion(list, "property missing"),
            "property missing = ${1:value}");
  const auto reference =
      completion(world, "xsmpsed",
                 "schedule Demo\ntask Main on "
                 "demo.MissingPropertyType\n{\nproperty <|>\n}");
  EXPECT_EQ(insertion(reference, "missing"), "missing = ${1:value}");
}

TEST(DeploymentCompletionProvider, ConfigurationSnippetsFieldsAndTypedValues) {
  auto world = workspace();
  auto root = completion(world, "xsmpcfg", "configuration Demo\n<|>");
  EXPECT_TRUE(has(root, "Root Component Configuration"));
  EXPECT_TRUE(has(root, "Include Configuration"));
  auto fields = completion(
      world, "xsmpcfg", "configuration Demo\n/Root: demo.Root\n{\n <|>\n}\n");
  EXPECT_TRUE(has(fields, "Component Configuration"));
  EXPECT_FALSE(has(fields, "Root Component Configuration"));
  EXPECT_TRUE(has(fields, "flag"));
  EXPECT_NE(insertion(fields, "flag").find("flag = "), std::string::npos);
  EXPECT_NE(insertion(fields, "flags").find("[false, false]"),
            std::string::npos);
  auto values =
      completion(world, "xsmpcfg",
                 "configuration Demo\n/Root: demo.Root\n{\n flag = <|>\n}\n");
  EXPECT_TRUE(has(values, "false"));
  EXPECT_TRUE(has(values, "true"));
  auto structure = completion(
      world, "xsmpcfg",
      "configuration Demo\n/Root: demo.Root\n{\n state = {\n <|>\n}\n}\n");
  EXPECT_TRUE(has(structure, "count"));
  EXPECT_NE(insertion(structure, "count").find("count = "), std::string::npos);
  auto number = completion(world, "xsmpcfg",
                           "configuration Demo\n/Root: demo.Root\n{\n state = "
                           "{\n count = <|>\n}\n}\n");
  EXPECT_TRUE(has(number, "Default Value"));
  EXPECT_FALSE(has(number, "false"));
  EXPECT_FALSE(has(number, "true"));
}

TEST(DeploymentCompletionProvider, AssemblySubModelsAndInvocationEnrichment) {
  auto world = workspace();
  auto list = completion(world, "xsmpasb",
                         "assembly Demo\nRoot: demo.Root\n{\n <|>\n}\n");
  EXPECT_TRUE(has(list, "Sub Model Instance"));
  EXPECT_TRUE(has(list, "Field Link"));
  EXPECT_TRUE(has(list, "Interface Link"));
  EXPECT_TRUE(has(list, "child += Child"));
  EXPECT_FALSE(has(list, "field link outValue -> Child.inValue"));
  auto prefixed = completion(world, "xsmpasb",
                             "assembly Demo\nRoot: demo.Root\n{\n ch<|>\n}\n");
  EXPECT_TRUE(has(prefixed, "child += Child"));
  auto call = completion(
      world, "xsmpasb",
      "assembly Demo\nRoot: demo.Root {}\nconfigure /Root\n{\n call <|>\n}\n");
  EXPECT_NE(insertion(call, "apply").find("apply("), std::string::npos);
  auto types = completion(world, "xsmpasb", "assembly Demo\nRoot: <|>\n{ }\n");
  EXPECT_TRUE(has(types, "demo.Root"));
  EXPECT_FALSE(has(types, "DemoAsm"));
}

TEST(DeploymentCompletionProvider, LinkBaseAnchorsAndRelativeMemberPaths) {
  auto world = workspace();
  auto root = completion(world, "xsmplnk", "link Demo for DemoAsm\n<|>");
  EXPECT_TRUE(has(root, "Root Component Link Base"));
  auto list =
      completion(world, "xsmplnk", "link Demo for DemoAsm\n/\n{\n <|>\n}\n");
  EXPECT_TRUE(has(list, "Component Link Base"));
  EXPECT_FALSE(has(list, "Root Component Link Base"));
  EXPECT_TRUE(has(list, "Child"));
  EXPECT_TRUE(has(list, "Field Link"));
  auto refs = completion(world, "xsmplnk",
                         "link Demo for DemoAsm\n/\n{\n interface link <|> -> "
                         "Child:backLogger\n}\n");
  EXPECT_TRUE(has(refs, "logger"));
  auto back = completion(
      world, "xsmplnk",
      "link Demo for DemoAsm\n/\n{\n interface link logger -> Child:<|>\n}\n");
  EXPECT_TRUE(has(back, "backLogger"));
}

TEST(DeploymentCompletionProvider,
     ScheduleEventsCallsValuesAndTemplateTaskArguments) {
  auto world = workspace();
  auto root = completion(world, "xsmpsed", "schedule Demo\n<|>");
  EXPECT_TRUE(has(root, "Task"));
  EXPECT_TRUE(has(root, "Event Epoch"));
  EXPECT_TRUE(has(root, "Event Global Trigger"));
  auto call =
      completion(world, "xsmpsed",
                 "schedule Demo\ntask Main on demo.Root\n{\n call <|>\n}\n");
  EXPECT_NE(insertion(call, "apply").find("apply("), std::string::npos);
  auto values = completion(
      world, "xsmpsed",
      "schedule Demo\ntask Main on demo.Root\n{\n property mode = <|>\n}\n");
  EXPECT_TRUE(has(values, "demo.Mode.Nominal"));
  EXPECT_TRUE(has(values, "demo.Mode.Safe"));
  auto execute = completion(
      world, "xsmpsed",
      "schedule <Root: string = \"child\", Cycles: int32 = 3> Demo\ntask Main "
      "on demo.Root\n{\n execute <|>\n}\ntask Worker on demo.Child {}\n");
  EXPECT_NE(insertion(execute, "Worker").find("Worker<"), std::string::npos);
  EXPECT_NE(insertion(execute, "Worker").find("Root = "), std::string::npos);
  EXPECT_NE(insertion(execute, "Worker").find("Cycles = "), std::string::npos);
  auto body = completion(world, "xsmpsed",
                         "schedule Demo\ntask Main on demo.Root\n{\n "
                         "<|>\n}\ntask Worker on demo.Child {}\n");
  EXPECT_TRUE(has(body, "call apply"));
  EXPECT_TRUE(has(body, "property mode"));
  EXPECT_TRUE(has(body, "trig step"));
  EXPECT_TRUE(has(body, "execute Worker"));
}

// Each end of a link is a choice of its own, so what is offered grows with the
// members and not with their pairs.
TEST(DeploymentCompletionProvider, EachEndOfALinkIsAChoice) {
  auto world = workspace();
  const auto snippet = [&](std::string_view id, std::string text,
                           std::string_view label) {
    const auto list = completion(world, id, std::move(text));
    for (const auto& item : list.items)
      for (const auto prefix : {"field link ", "event link ", "transfer "})
        EXPECT_FALSE(item.label.starts_with(prefix)) << item.label;
    return insertion(list, label);
  };
  const auto assembly = "assembly Demo\nRoot: demo.Root\n{\n <|>\n}\n";
  EXPECT_EQ(snippet("xsmpasb", assembly, "Field Link"),
            "field link ${1|outValue|} -> ${2|inValue|}");
  EXPECT_EQ(snippet("xsmpasb", assembly, "Event Link"),
            "event link ${1:owner} -> ${2:client}");
  EXPECT_EQ(snippet("xsmplnk", "link Demo for DemoAsm\n/\n{\n <|>\n}\n",
                    "Field Link"),
            "field link ${1|outValue,Child.outValue|} -> "
            "${2|inValue,Child.inValue|}");
  EXPECT_EQ(snippet("xsmpsed", "schedule Demo\ntask Main on demo.Root\n{\n <|>\n}\n",
                    "Field Transfer"),
            "transfer ${1|outValue|} -> ${2|inValue|}");
  EXPECT_EQ(snippet("xsmpsed", "schedule Demo\ntask Main\n{\n <|>\n}\n",
                    "Field Transfer"),
            "transfer ${1:output} -> ${2:input}");
}

TEST(DeploymentCompletionProvider, TemplateParametersAreNotOfferedAsTypes) {
  auto world = workspace();
  const auto list = completion(
      world, "xsmpasb",
      "assembly <Lane = \"A\"> Asm\nRoot: demo.Root\n{\n    <|>\n}\n");
  const auto item = std::ranges::find_if(list.items, [](const auto& candidate) {
    return candidate.label == "Sub Model Instance";
  });
  ASSERT_NE(item, list.items.end());
  EXPECT_EQ(item->insertText.value_or("").find("Lane"), std::string::npos);

  const auto tasks =
      completion(world, "xsmpsed",
                 "schedule <Lane = \"A\"> S\ntask T on demo.Root\n{\n}\n<|>\n");
  const auto event = std::ranges::find_if(
      tasks.items,
      [](const auto& candidate) { return candidate.label == "Event Mission"; });
  ASSERT_NE(event, tasks.items.end());
  EXPECT_EQ(event->insertText.value_or("").find("Lane"), std::string::npos);
}

// What a link offers grows with the members of the instances, not with their
// pairs, and still follows what is typed.
TEST(DeploymentCompletionProvider, LinksGrowWithTheMembersAndFollowWhatIsTyped) {
  auto world = workspace();
  std::string assembly = "assembly Big\nRoot: demo.Root\n{\n";
  for (auto i = 0; i < 40; ++i)
    assembly += "  child += Kid" + std::to_string(i) + ": demo.Child\n";
  (void)xsmp::parse(world, "xsmpasb", assembly + "}\n",
                    "file:///completion/smdl/Big.xsmpasb");
  const auto head = std::string("link L for Big\n/\n{\n    ");
  const auto all = completion(world, "xsmplnk", head + "<|>\n}\n");
  EXPECT_LT(all.items.size(), 100u);
  const auto field = insertion(all, "Field Link");
  EXPECT_NE(field.find(",Kid39.outValue|}"), std::string::npos);
  EXPECT_EQ(std::ranges::count(field, ','), 2 * 40);
  EXPECT_TRUE(
      completion(world, "xsmplnk", head + "zzzq<|>\n}\n").items.empty());
  EXPECT_TRUE(
      has(completion(world, "xsmplnk", head + "Kid39<|>\n}\n"), "Kid39"));
}

TEST(DeploymentCompletionProvider, DefaultValuesStayShort) {
  auto world = workspace();
  (void)xsmp::parse(
      world, "xsmpcat",
      "catalogue Big\nnamespace big\n{\n"
      "public array Row = Smp.Int32[1000]\npublic array Grid = Row[1000]\n"
      "public struct Holder { field Grid grid }\n"
      "public model M { field Grid grid field Holder holder }\n}\n",
      "file:///completion/smdl/Big.xsmpcat");
  const auto list = completion(
      world, "xsmpcfg", "configuration C\n/Root: big.M\n{\n    <|>\n}\n");
  for (const std::string_view label : {"grid", "holder"}) {
    const auto item = std::ranges::find_if(
        list.items, [&](const auto& it) { return it.label == label; });
    ASSERT_NE(item, list.items.end()) << label;
    EXPECT_LT(std::get<::lsp::TextEdit>(*item->textEdit).newText.size(), 6000u);
  }
}

TEST(DeploymentCompletionProvider, AssertionContracts) {
  using namespace xsmp;
  const auto records = ::lsp::json::parse(
      readText(std::filesystem::path(XSMP_FIXTURES_DIR)
               / "lsp/deployment-completion-contracts.json"));
  const auto* cases = records.object().find("cases");
  ASSERT_TRUE(cases && cases->isArray());
  ASSERT_EQ(cases->array().size(), 35u);
  for (const auto& entry : cases->array()) {
    const auto& record = entry.object();
    SCOPED_TRACE(stringAt(record, "name"));
    auto world = workspace();
    for (const auto& background : record.find("documents")->array()) {
      const auto uri = stringAt(background.object(), "uri");
      const auto extension = std::filesystem::path(uri).extension().string();
      const auto language = uri.ends_with("xsmp.project")
                                ? std::string("xsmpproject")
                                : extension.substr(1);
      (void)xsmp::parse(world, language, stringAt(background.object(), "text"),
                        uri);
    }
    const auto uri = stringAt(record, "uri");
    ::lsp::CompletionList list;
    std::shared_ptr<cursiv::Document> document;
    (void)cursiv::test::expectCompletion(
        world.language(stringAt(record, "language")),
        cursiv::test::ExpectedCompletionCallback{
            .text = stringAt(record, "text"),
            .assertion =
                [&](const ::lsp::CompletionList& answer) {
                  list = answer;
                  document = world.shared().documents->getDocument(uri);
                },
            .parseOptions = xsmp::documentAt(uri),
            .disposeAfterCheck = true});
    ASSERT_TRUE(document);
    for (const auto& requirement : record.find("assertions")->array()) {
      const auto& assertion = requirement.object();
      const auto kind = stringAt(assertion, "kind"),
                 label = stringAt(assertion, "label"),
                 expected = stringAt(assertion, "value");
      SCOPED_TRACE(kind + ": " + label + " / " + expected);
      if (kind == "present")
        EXPECT_TRUE(has(list, label));
      else if (kind == "absent")
        EXPECT_FALSE(has(list, label));
      else if (kind == "insertContains")
        EXPECT_NE(insertion(list, label).find(expected), std::string::npos);
      else if (kind == "insertNotContains")
        EXPECT_EQ(insertion(list, label).find(expected), std::string::npos);
      else if (kind == "detailContains") {
        const auto found = std::ranges::find_if(
            list.items, [&](const auto& item) { return item.label == label; });
        ASSERT_NE(found, list.items.end());
        EXPECT_NE(found->detail.value_or("").find(expected), std::string::npos);
      }
    }
  }
}
