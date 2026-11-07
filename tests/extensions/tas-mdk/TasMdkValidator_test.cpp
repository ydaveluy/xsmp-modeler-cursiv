#include <xsmp/tas-mdk/TasMdkValidator.hpp>

#include "support/DiagnosticTestSupport.hpp"
#include "support/LspTestSupport.hpp"

TEST(TasMdkValidator, TasValidationChecksFieldDirectionAndNaming) {
  auto world = xsmp::TestWorkspace::withShared<xsmp::XsmpSharedServices>();
  auto& shared = world.shared();
  shared.projectManager = std::make_unique<xsmp::ProjectManager>(shared);
  auto language = xsmp::createXsmpcatServices(shared);
  xsmp::registerTasMdkValidationChecks(*language, cursiv::fast);
  world.registerLanguage(std::move(language));
  const auto result = cursiv::test::validationHelper(
      world.language("xsmpcat"),
      "catalogue Demo\nnamespace n { primitive P model M { input output field "
      "P both input field P value } }\n");
  EXPECT_TRUE(std::ranges::any_of(result.diagnostics, [](const auto& d) {
    return d.message == "A field cannot be both an input and an output.";
  }));
  EXPECT_TRUE(std::ranges::any_of(result.diagnostics, [](const auto& d) {
    return d.message == "The name of an input field must start with 'inp_'.";
  }));
}

TEST(TasMdkValidator, TasValidationChecksBaseModelAndOperationParameters) {
  auto world = xsmp::TestWorkspace::withShared<xsmp::XsmpSharedServices>();
  auto& shared = world.shared();
  shared.projectManager = std::make_unique<xsmp::ProjectManager>(shared);
  auto language = xsmp::createXsmpcatServices(shared);
  xsmp::registerTasMdkValidationChecks(*language, cursiv::fast);
  world.registerLanguage(std::move(language));
  const auto result = cursiv::test::validationHelper(
      world.language("xsmpcat"),
      "catalogue Demo\nnamespace n { struct S {} model M { def void "
      "ope_run(in Missing a, in S b) } }\n");
  const auto has = [&](std::string_view message) {
    return std::ranges::count_if(result.diagnostics, [&](const auto& d) {
      return d.message == message;
    });
  };
  EXPECT_EQ(has("The Model M must extend 'TasMdk.Model' or one of its "
                "subclasses."),
            1);
  // The unresolved type of `a` is only reported as unresolved.
  EXPECT_EQ(has("A parameter of type Structure is not publicable."), 1);
  EXPECT_EQ(has("Could not resolve reference to Type named 'Missing'."), 1);
  EXPECT_EQ(std::ranges::count_if(result.diagnostics,
                                  [](const auto& d) {
                                    return d.message.find("not publicable")
                                           != std::string::npos;
                                  }),
            1);
}

TEST(TasMdkValidator, AnEventSinkTakesNoPayload) {
  auto world = xsmp::TestWorkspace::withShared<xsmp::XsmpSharedServices>();
  auto& shared = world.shared();
  shared.projectManager = std::make_unique<xsmp::ProjectManager>(shared);
  auto language = xsmp::createXsmpcatServices(shared);
  xsmp::registerTasMdkValidationChecks(*language, cursiv::fast);
  world.registerLanguage(std::move(language));
  // The event type may come from a project without the profile, where nothing
  // reports its payload: the sink itself does.
  const auto result = cursiv::test::validationHelper(
      world.language("xsmpcat"),
      "catalogue Demo\nnamespace n { primitive P event Payload extends P event "
      "Empty model M { eventsink Payload esi_payload eventsink Empty "
      "esi_empty } }\n");
  EXPECT_EQ(std::ranges::count_if(result.diagnostics,
                                  [](const auto& d) {
                                    return d.message
                                           == "An EventSink must be of type "
                                              "void.";
                                  }),
            1);
}

TEST(TasMdkValidator, TasValidationNeedsAProjectThatSelectsTheProfile) {
  for (const std::string profile :
       {"tas-mdk", "org.eclipse.xsmp.profile.tas-mdk", "TasMdk"}) {
    SCOPED_TRACE(profile);
    auto world = xsmp::TestWorkspace::withShared<xsmp::XsmpSharedServices>();
    auto& shared = world.shared();
    shared.projectManager = std::make_unique<xsmp::ProjectManager>(shared);
    auto language = xsmp::createXsmpcatServices(shared);
    shared.contributionAliases["org.eclipse.xsmp.profile.tas-mdk"] = "tas-mdk";
    xsmp::registerTasMdkValidationChecks(
        *language, xsmp::contributionCategory(shared, "tas-mdk"));
    world.registerLanguage(std::move(language));
    world.registerLanguage(xsmp::createXsmpprojectServices(shared));
    auto project = shared.documents->createDocument(
        "file:///tas/xsmp.project",
        "project 'Tas' source 'src' profile '" + profile + "'");
    auto catalogue = shared.documents->createDocument(
        "file:///tas/src/Demo.xsmpcat",
        "catalogue Demo namespace n { model M {} }");
    xsmp::buildDocuments(*shared.documentBuilder,
                         std::vector{project, catalogue});
    EXPECT_EQ(
        std::ranges::any_of(catalogue->diagnostics,
                            [](const auto& d) {
                              return d.message.find("must extend 'TasMdk.")
                                     != std::string::npos;
                            }),
        profile != "TasMdk");
  }
}
