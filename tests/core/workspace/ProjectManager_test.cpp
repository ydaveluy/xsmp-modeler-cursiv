#include <gtest/gtest.h>

#include <algorithm>
#include <cursiv/services/Diagnostic.hpp>
#include <cursiv/workspace/Document.hpp>
#include <cursiv/workspace/DocumentBuilder.hpp>
#include <cursiv/workspace/Documents.hpp>
#include <memory>
#include <mutex>
#include <set>
#include <span>
#include <stop_token>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
#include <xsmp/Builtins.hpp>
#include <xsmp/DeploymentLspModule.hpp>
#include <xsmp/SharedLspServices.hpp>
#include <xsmp/XsmpprojectLspModule.hpp>
#include <xsmp/ast.hpp>
#include <xsmp/utils/TreeUtils.hpp>
#include <xsmp/utils/XsmpUtils.hpp>
#include <xsmp/workspace/ProjectManager.hpp>

#include "support/DiagnosticTestSupport.hpp"

namespace {

using Doc = std::shared_ptr<cursiv::Document>;

// Registers xsmpcat and xsmp.project on a `TestWorkspace` built from the real
// `xsmp::XsmpSharedServices`, with its `projectManager`, so the project
// visibility filtering of the global scope is exercised end to end.
struct Fixture {
  xsmp::TestWorkspace ws =
      xsmp::TestWorkspace::withShared<xsmp::XsmpSharedServices>();

  Fixture() {
    auto& shared = ws.shared<xsmp::XsmpSharedServices>();
    shared.projectManager = std::make_unique<xsmp::ProjectManager>(shared);
    shared.serviceRegistry->registerServices(
        xsmp::createXsmpcatServices(shared));
    shared.serviceRegistry->registerServices(
        xsmp::createXsmpprojectServices(shared));
    xsmp::loadXsmpcatBuiltins(shared);
  }

  Doc add(std::string uri, std::string text) {
    return ws.shared<xsmp::XsmpSharedServices>().documents->createDocument(
        std::move(uri), std::move(text));
  }

  void build(const std::vector<Doc>& docs, bool validate = false) {
    cursiv::BuildOptions options;
    options.validation = validate;
    xsmp::buildDocuments(*ws.shared<xsmp::XsmpSharedServices>().documentBuilder,
                         docs, options);
  }
};

bool hasWarning(const Doc& doc, std::string_view needle) {
  for (const auto& diagnostic : doc->diagnostics) {
    if (diagnostic.severity == cursiv::DiagnosticSeverity::Warning
        && diagnostic.message.find(needle) != std::string::npos) {
      return true;
    }
  }
  return false;
}

bool hasError(const Doc& doc, std::string_view needle) {
  for (const auto& diagnostic : doc->diagnostics) {
    if (diagnostic.severity == cursiv::DiagnosticSeverity::Error
        && diagnostic.message.find(needle) != std::string::npos) {
      return true;
    }
  }
  return false;
}

const xsmp::Field* firstField(const Doc& document) {
  using namespace xsmp;
  const auto* catalogue =
      cursiv::ast_cast<const Catalogue>(document->parseResult.value);
  if (catalogue == nullptr || catalogue->elements.empty()) {
    return nullptr;
  }
  const auto* ns = catalogue->elements[0];
  if (ns == nullptr || ns->elements.empty()) {
    return nullptr;
  }
  const auto* structure = cursiv::ast_cast<const Structure>(ns->elements[0]);
  if (structure == nullptr || structure->elements.empty()) {
    return nullptr;
  }
  return cursiv::ast_cast<const Field>(structure->elements[0]);
}

}  // namespace

TEST(ProjectManager, EffectivePublicStructAndInterfaceConstantsAreExported) {
  Fixture fx;
  auto project = fx.add("file:///p/xsmp.project", "project 'P' source 'src'");
  auto library =
      fx.add("file:///p/src/lib.xsmpcat", R"(catalogue Library namespace n {
    struct S { private constant Smp.Int32 a = 1 }
    interface I { protected constant Smp.Int32 b = 2 }
  })");
  auto user =
      fx.add("file:///p/src/user.xsmpcat", R"(catalogue User namespace n {
    struct K { constant Smp.Int32 x = n.S.a constant Smp.Int32 y = n.I.b }
  })");
  fx.build({project, library, user});
  std::size_t references = 0;
  for (const auto* node : xsmp::descendantsOf(*user->parseResult.value))
    if (const auto* reference =
            cursiv::ast_cast<xsmp::NamedElementReference>(node)) {
      ASSERT_TRUE(reference->value) << reference->value.refText;
      ++references;
    }
  EXPECT_EQ(references, 2u);
}

TEST(ProjectManager, SourcePathsWithUriPunctuationStillOwnAndExposeDocuments) {
  Fixture fx;
  auto project =
      fx.add("file:///p/xsmp.project", "project 'P' source 'src #?%2F'");
  auto first = fx.add("file:///p/src%20%23%3F%252F/a.xsmpcat",
                      "catalogue A namespace n { primitive P }");
  auto second =
      fx.add("file:///p/src%20%23%3F%252F/b.xsmpcat",
             "catalogue B namespace n { struct S { field n.P value } }");
  fx.build({project, first, second});
  auto& manager = *fx.ws.shared().projectManager;
  EXPECT_EQ(manager.getProject(*first), project->parseResult.value);
  EXPECT_TRUE(manager.getVisibleDocumentIds(*second)->contains(first->id));
  ASSERT_NE(firstField(second), nullptr);
  EXPECT_NE(firstField(second)->type.get(), nullptr);
}

// A project's `.xsmpcat` document sees types declared by a project it
// `dependency`-declares (resolved through its `source` folder), via
// XsmpcatScopeProvider's project-visibility-filtered global scope.
TEST(ProjectManager, DependentProjectResolvesType) {
  Fixture fx;
  auto projectA =
      fx.add("file:///a/xsmp.project", "project 'A'\nsource 'src'\n");
  auto typeA = fx.add("file:///a/src/lib.xsmpcat",
                      "catalogue lib\nnamespace n { primitive PA }\n");
  auto projectB = fx.add("file:///b/xsmp.project",
                         "project 'B'\nsource 'src'\ndependency 'A'\n");
  auto typeB = fx.add("file:///b/src/lib.xsmpcat",
                      "catalogue lib\n"
                      "namespace n { struct S { field n.PA x } }\n");
  fx.build({projectA, typeA, projectB, typeB});

  const auto* field = firstField(typeB);
  ASSERT_NE(field, nullptr);
  ASSERT_TRUE(field->type) << "n.PA should resolve: B depends on A";
  EXPECT_EQ(field->type.get()->name, "PA");
}

// A project's `.xsmpcat` document does NOT see types declared by a project it
// has no `dependency` on, even though both are in the same workspace.
TEST(ProjectManager, UnrelatedProjectDoesNotResolveType) {
  Fixture fx;
  auto projectA =
      fx.add("file:///a/xsmp.project", "project 'A'\nsource 'src'\n");
  auto typeA = fx.add("file:///a/src/lib.xsmpcat",
                      "catalogue lib\nnamespace n { primitive PA }\n");
  auto projectC =
      fx.add("file:///c/xsmp.project", "project 'C'\nsource 'src'\n");
  auto typeC = fx.add("file:///c/src/lib.xsmpcat",
                      "catalogue lib\n"
                      "namespace n { struct S { field n.PA x } }\n");
  fx.build({projectA, typeA, projectC, typeC});

  const auto* field = firstField(typeC);
  ASSERT_NE(field, nullptr);
  EXPECT_FALSE(field->type) << "n.PA must stay unresolved: C has no "
                               "dependency on A";
}

// Editing an `xsmp.project` to drop a `dependency` must invalidate the
// already-resolved cross-project references it made possible, even though the
// referring `.xsmpcat` document's own text never changed. A catalogue's
// reference points at ANOTHER catalogue (where the type is declared), never at
// the `xsmp.project` file whose `dependency`/`source` graph made that
// catalogue visible, so the reference index alone cannot see this edit as
// relevant.
TEST(ProjectManager, RemovingDependencyInvalidatesStaleResolvedReferences) {
  Fixture fx;
  auto projectA =
      fx.add("file:///a/xsmp.project", "project 'A'\nsource 'src'\n");
  auto typeA = fx.add("file:///a/src/lib.xsmpcat",
                      "catalogue lib\nnamespace n { primitive PA }\n");
  auto projectB = fx.add("file:///b/xsmp.project",
                         "project 'B'\nsource 'src'\ndependency 'A'\n");
  auto typeB = fx.add("file:///b/src/lib.xsmpcat",
                      "catalogue lib\n"
                      "namespace n { struct S { field n.PA x } }\n");
  fx.build({projectA, typeA, projectB, typeB});

  ASSERT_TRUE(firstField(typeB)->type)
      << "n.PA should resolve before the edit: B depends on A";

  // Edits `xsmp.project` in place (same DocumentId, like a textDocument/
  // didChange), not as a delete and recreate.
  xsmp::parse(fx.ws, "xsmpproject", "project 'B'\nsource 'src'\n",
              std::string("file:///b/xsmp.project"));

  EXPECT_FALSE(firstField(typeB)->type)
      << "n.PA must become unresolved once B no longer depends on A";
}

// A build keeps what it asks for, with a project among its documents: nothing
// validates a catalogue when the build asks for no validation, whether the
// catalogue is new or the new project relinks it. `P` has no `@uuid`, which
// validation would report.
// What a project sees survives an edit of a catalogue, but not a new document
// or an edit of the project.
TEST(ProjectManager, AProjectSeesWhatChangesAndKeepsTheRest) {
  Fixture fx;
  auto project =
      fx.add("file:///p/xsmp.project", "project 'P'\nsource 'src'\n");
  auto first = fx.add("file:///p/src/a.xsmpcat", "catalogue a\n");
  fx.build({project, first});
  const auto& manager =
      *fx.ws.shared<xsmp::XsmpSharedServices>().projectManager;
  EXPECT_NE(manager.getProject(*first), nullptr);

  first = xsmp::parse(fx.ws, "xsmpcat", "catalogue a2\n",
                      std::string("file:///p/src/a.xsmpcat"));
  EXPECT_TRUE(manager.getVisibleDocumentIds(*first)->contains(first->id));

  auto second = fx.add("file:///p/src/b.xsmpcat", "catalogue b\n");
  fx.build({second});
  EXPECT_TRUE(manager.getVisibleDocumentIds(*first)->contains(second->id));

  project = xsmp::parse(fx.ws, "xsmpproject", "project 'P'\nsource 'other'\n",
                        std::string("file:///p/xsmp.project"));
  EXPECT_EQ(manager.getProject(*first), nullptr);
  EXPECT_FALSE(manager.getVisibleDocumentIds(*first)->contains(second->id));
}

TEST(ProjectManager, ABuildWithoutValidationValidatesNoCatalogueOfAProject) {
  Fixture fx;
  auto project =
      fx.add("file:///p/xsmp.project", "project 'P'\nsource 'src'\n");
  auto catalogue = fx.add("file:///p/src/c.xsmpcat",
                          "catalogue c\nnamespace n { public primitive P }\n");
  fx.build({project, catalogue});
  EXPECT_EQ(catalogue->state.load(), cursiv::DocumentState::IndexedReferences);
  EXPECT_TRUE(catalogue->diagnostics.empty());

  fx.build({catalogue}, /*validate=*/true);
  ASSERT_TRUE(hasError(catalogue, "Missing Type UUID."));

  auto nested =
      fx.add("file:///p/src/inner/xsmp.project", "project 'Q'\nsource '.'\n");
  fx.build({nested, catalogue});
  EXPECT_EQ(catalogue->state.load(), cursiv::DocumentState::IndexedReferences);
  EXPECT_TRUE(catalogue->diagnostics.empty());
}

// A new project says what a catalogue outside the build now sees, and the same
// build relinks it.
TEST(ProjectManager, ANewProjectRelinksACatalogueOutsideTheBuild) {
  Fixture fx;
  auto projectA =
      fx.add("file:///a/xsmp.project", "project 'A'\nsource 'src'\n");
  auto library = fx.add("file:///a/src/lib.xsmpcat",
                        "catalogue lib\nnamespace n { primitive PA }\n");
  auto user = fx.add("file:///b/src/use.xsmpcat",
                     "catalogue use\n"
                     "namespace m { struct S { field n.PA x } }\n");
  fx.build({projectA, library, user});
  ASSERT_FALSE(firstField(user)->type);

  auto projectB = fx.add("file:///b/xsmp.project",
                         "project 'B'\nsource 'src'\ndependency 'A'\n");
  fx.build({projectB});
  EXPECT_TRUE(firstField(user)->type);
}

// A deleted project leaves what it gave to its catalogues.
TEST(ProjectManager, DeletingAProjectRelinksItsCatalogues) {
  Fixture fx;
  auto projectA =
      fx.add("file:///a/xsmp.project", "project 'A'\nsource 'src'\n");
  auto library = fx.add("file:///a/src/lib.xsmpcat",
                        "catalogue lib\nnamespace n { primitive PA }\n");
  auto projectB = fx.add("file:///b/xsmp.project",
                         "project 'B'\nsource 'src'\ndependency 'A'\n");
  auto user = fx.add("file:///b/src/use.xsmpcat",
                     "catalogue use\n"
                     "namespace m { struct S { field n.PA x } }\n");
  fx.build({projectA, library, projectB, user});
  ASSERT_TRUE(firstField(user)->type);

  const std::string_view gone = "file:///b/xsmp.project";
  fx.ws.shared<xsmp::XsmpSharedServices>().documentBuilder->update(
      {}, std::span(&gone, 1));
  EXPECT_FALSE(firstField(user)->type);
}

// A project that omits `using '<standard>'` has an empty `Project::standard`.
// It must still default to ECSS_SMP_2020 when deciding which builtin
// catalogues are visible, since every builtin carries an explicit standard.
TEST(ProjectManager, ProjectWithoutUsingClauseStillSeesBuiltins) {
  Fixture fx;
  auto projectA =
      fx.add("file:///a/xsmp.project", "project 'A'\nsource 'src'\n");
  auto typeA = fx.add("file:///a/src/lib.xsmpcat",
                      "catalogue lib\n"
                      "namespace n { struct S { field Bool c } }\n");
  fx.build({projectA, typeA});

  const auto* field = firstField(typeA);
  ASSERT_NE(field, nullptr);
  ASSERT_TRUE(field->type) << "Bool should resolve against the auto-loaded "
                              "ECSS_SMP_2020 builtins even without `using`";
  EXPECT_EQ(field->type.get()->name, "Bool");
}

// A Reference may target any Component from ECSS_SMP_2025 on; in ECSS_SMP_2020
// it shall target an Interface. The standard comes from the project, or from
// the name of a builtin catalogue.
TEST(ProjectManager, ReferenceTargetDependsOnTheStandard) {
  const std::string body =
      "catalogue lib namespace n { model M {} model R { reference M target } }";
  for (const std::string standard : {"ECSS_SMP_2020", "ECSS_SMP_2025"}) {
    SCOPED_TRACE(standard);
    const auto error = standard == "ECSS_SMP_2020";
    Fixture project;
    auto descriptor =
        project.add("file:///p/xsmp.project",
                    "project 'P' using '" + standard + "'\nsource 'src'\n");
    auto catalogue = project.add("file:///p/src/lib.xsmpcat", body);
    project.build({descriptor, catalogue}, /*validate=*/true);
    EXPECT_EQ(hasError(catalogue, "shall target an Interface"), error);
    Fixture builtin;
    auto probe = builtin.add("xsmp:/probe%40" + standard + ".xsmpcat", body);
    builtin.build({probe}, /*validate=*/true);
    EXPECT_EQ(hasError(probe, "shall target an Interface"), error);
  }
}

// A catalogue outside every `xsmp.project`'s directory/source folders warns.
TEST(ProjectManager, CatalogueNotContainedInProjectWarns) {
  Fixture fx;
  auto projectA =
      fx.add("file:///a/xsmp.project", "project 'A'\nsource 'src'\n");
  auto stray = fx.add("file:///stray/lib.xsmpcat", "catalogue lib\n");
  fx.build({projectA, stray}, /*validate=*/true);

  EXPECT_TRUE(hasWarning(stray, "not contained in a project"));
}

// A catalogue under its project's `source` folder does not warn.
TEST(ProjectManager, CatalogueContainedInProjectHasNoWarning) {
  Fixture fx;
  auto projectA =
      fx.add("file:///a/xsmp.project", "project 'A'\nsource 'src'\n");
  auto typeA = fx.add("file:///a/src/lib.xsmpcat", "catalogue lib\n");
  fx.build({projectA, typeA}, /*validate=*/true);

  EXPECT_FALSE(hasWarning(typeA, "not contained in a project"));
}

// Two catalogues in the same project, each referencing a type declared by the
// other, form a 2-cycle. Both documents get the diagnostic, one per
// participant.
TEST(ProjectManager,
     TwoCataloguesCircularlyReferencingEachOtherReportCircularDependency) {
  Fixture fx;
  auto projectP =
      fx.add("file:///p/xsmp.project", "project 'P'\nsource 'src'\n");
  auto catA =
      fx.add("file:///p/src/a.xsmpcat",
             "catalogue a\n"
             "namespace n { primitive PA struct SA { field n2.PB refToB } }\n");
  auto catB =
      fx.add("file:///p/src/b.xsmpcat",
             "catalogue b\n"
             "namespace n2 { primitive PB struct SB { field n.PA refToA } }\n");
  fx.build({projectP, catA, catB}, /*validate=*/true);

  EXPECT_TRUE(
      hasError(catA, "Catalogues shall not have circular dependencies"));
  EXPECT_TRUE(
      hasError(catB, "Catalogues shall not have circular dependencies"));
  for (const auto& catalogue : {catA, catB})
    for (const auto& diagnostic : catalogue->diagnostics)
      if (diagnostic.message
          == "Catalogues shall not have circular dependencies.")
        EXPECT_EQ(diagnostic.relatedInformation.size(), 1u);
}

// A 3-catalogue cycle (A -> B -> C -> A) is detected too.
TEST(ProjectManager, ThreeCatalogueCycleIsDetected) {
  Fixture fx;
  auto projectP =
      fx.add("file:///p/xsmp.project", "project 'P'\nsource 'src'\n");
  auto catA =
      fx.add("file:///p/src/a.xsmpcat",
             "catalogue a\n"
             "namespace n { primitive PA struct SA { field n2.PB refToB } }\n");
  auto catB = fx.add(
      "file:///p/src/b.xsmpcat",
      "catalogue b\n"
      "namespace n2 { primitive PB struct SB { field n3.PC refToC } }\n");
  auto catC =
      fx.add("file:///p/src/c.xsmpcat",
             "catalogue c\n"
             "namespace n3 { primitive PC struct SC { field n.PA refToA } }\n");
  fx.build({projectP, catA, catB, catC}, /*validate=*/true);

  EXPECT_TRUE(
      hasError(catA, "Catalogues shall not have circular dependencies"));
  EXPECT_TRUE(
      hasError(catB, "Catalogues shall not have circular dependencies"));
  EXPECT_TRUE(
      hasError(catC, "Catalogues shall not have circular dependencies"));
  for (const auto& catalogue : {catA, catB, catC})
    for (const auto& diagnostic : catalogue->diagnostics)
      if (diagnostic.message
          == "Catalogues shall not have circular dependencies.")
        EXPECT_EQ(diagnostic.relatedInformation.size(), 2u);
}

// A references B but B never references A back: a plain dependency, not a
// cycle.
TEST(ProjectManager, OneDirectionalCatalogueReferenceIsNotACircularDependency) {
  Fixture fx;
  auto projectP =
      fx.add("file:///p/xsmp.project", "project 'P'\nsource 'src'\n");
  auto catA =
      fx.add("file:///p/src/a.xsmpcat",
             "catalogue a\n"
             "namespace n { primitive PA struct SA { field n2.PB refToB } }\n");
  auto catB = fx.add("file:///p/src/b.xsmpcat",
                     "catalogue b\nnamespace n2 { primitive PB }\n");
  fx.build({projectP, catA, catB}, /*validate=*/true);

  EXPECT_FALSE(
      hasError(catA, "Catalogues shall not have circular dependencies"));
  EXPECT_FALSE(
      hasError(catB, "Catalogues shall not have circular dependencies"));
}

// A catalogue whose types only reference each other within the same document
// never reports a circular dependency: a same-document reference is not an
// edge of the dependency graph.
TEST(ProjectManager,
     IsolatedCatalogueWithOnlyLocalReferencesHasNoCircularDependencyError) {
  Fixture fx;
  auto projectP =
      fx.add("file:///p/xsmp.project", "project 'P'\nsource 'src'\n");
  auto catA =
      fx.add("file:///p/src/a.xsmpcat",
             "catalogue a\n"
             "namespace n { primitive PA struct SA { field n.PA self } }\n");
  fx.build({projectP, catA}, /*validate=*/true);

  EXPECT_FALSE(
      hasError(catA, "Catalogues shall not have circular dependencies"));
}

// Breaking a cycle (removing the back-reference) must clear the diagnostic on
// revalidation. Catalogue B is edited in place (same DocumentId), then every
// document is built and validated again.
TEST(ProjectManager, BreakingCatalogueCycleClearsTheDiagnosticOnRevalidation) {
  Fixture fx;
  auto projectP =
      fx.add("file:///p/xsmp.project", "project 'P'\nsource 'src'\n");
  auto catA =
      fx.add("file:///p/src/a.xsmpcat",
             "catalogue a\n"
             "namespace n { primitive PA struct SA { field n2.PB refToB } }\n");
  auto catB =
      fx.add("file:///p/src/b.xsmpcat",
             "catalogue b\n"
             "namespace n2 { primitive PB struct SB { field n.PA refToA } }\n");
  fx.build({projectP, catA, catB}, /*validate=*/true);
  ASSERT_TRUE(
      hasError(catA, "Catalogues shall not have circular dependencies"));
  ASSERT_TRUE(
      hasError(catB, "Catalogues shall not have circular dependencies"));

  catB = xsmp::parse(fx.ws, "xsmpcat",
                     "catalogue b\nnamespace n2 { primitive PB }\n",
                     std::string("file:///p/src/b.xsmpcat"));
  fx.build({projectP, catA, catB}, /*validate=*/true);

  EXPECT_FALSE(
      hasError(catA, "Catalogues shall not have circular dependencies"));
  EXPECT_FALSE(
      hasError(catB, "Catalogues shall not have circular dependencies"));
}

// A namespace is flagged only for an element of its own document. The collision
// with a visible Type of another document is reported on the Type (see
// DuplicatedTypeNameAlsoDetectsCollisionWithNamespace).
TEST(ProjectManager,
     NamespaceCollisionWithATypeOfAnotherDocumentIsNotReported) {
  Fixture fx;
  auto projectA =
      fx.add("file:///a/xsmp.project", "project 'A'\nsource 'src'\n");
  auto typeA = fx.add("file:///a/src/lib.xsmpcat",
                      "catalogue lib\nnamespace n { primitive B }\n");
  auto projectB = fx.add("file:///b/xsmp.project",
                         "project 'B'\nsource 'src'\ndependency 'A'\n");
  auto nsB = fx.add("file:///b/src/lib.xsmpcat",
                    "catalogue lib\nnamespace n { namespace B {} }\n");
  fx.build({projectA, typeA, projectB, nsB}, /*validate=*/true);

  EXPECT_FALSE(hasError(nsB, "Duplicated name."));
}

// A namespace whose qualified name collides with nothing in another document
// stays clean.
TEST(ProjectManager, NamespaceDoesNotCollideWithUnrelatedTypeAcrossDocuments) {
  Fixture fx;
  auto projectA =
      fx.add("file:///a/xsmp.project", "project 'A'\nsource 'src'\n");
  auto typeA = fx.add("file:///a/src/lib.xsmpcat",
                      "catalogue lib\nnamespace n { primitive Other }\n");
  auto projectB = fx.add("file:///b/xsmp.project",
                         "project 'B'\nsource 'src'\ndependency 'A'\n");
  auto nsB = fx.add("file:///b/src/lib.xsmpcat",
                    "catalogue lib\nnamespace n { namespace B {} }\n");
  fx.build({projectA, typeA, projectB, nsB}, /*validate=*/true);

  EXPECT_FALSE(hasError(nsB, "Duplicated name."));
}

// The duplicate-name check of a Type also sees a colliding Namespace, not just
// another Type. Reverse topology from the test above: project X (with the
// Type) depends on project Y (with the Namespace), so X sees Y but Y does not
// see X.
TEST(ProjectManager, DuplicatedTypeNameAlsoDetectsCollisionWithNamespace) {
  Fixture fx;
  auto projectY =
      fx.add("file:///y/xsmp.project", "project 'Y'\nsource 'src'\n");
  auto nsY = fx.add("file:///y/src/lib.xsmpcat",
                    "catalogue lib\nnamespace n { namespace B {} }\n");
  auto projectX = fx.add("file:///x/xsmp.project",
                         "project 'X'\nsource 'src'\ndependency 'Y'\n");
  auto typeX = fx.add("file:///x/src/lib.xsmpcat",
                      "catalogue lib\nnamespace n { primitive B }\n");
  fx.build({projectY, nsY, projectX, typeX}, /*validate=*/true);

  EXPECT_TRUE(hasError(typeX, "Duplicated Type name."));
  EXPECT_FALSE(hasError(nsY, "Duplicated name."))
      << "Y cannot see X (no dependency in that direction): only the side "
         "that can see the collision reports it";
}

TEST(ProjectManager,
     SmpMirrorOwnershipAndDependencyVisibilityFollowTheXmlFile) {
  Fixture fx;
  auto projectA = fx.add("file:///a/xsmp.project",
                         "project 'A' using 'ECSS_SMP_2025'\nsource 'smdl'\n");
  auto mirror = fx.add(
      "xsmp-smp:///a/smdl/foundation.smpcat",
      "catalogue Foundation\nnamespace foundation { struct Payload {} }\n");
  auto projectB = fx.add(
      "file:///b/xsmp.project",
      "project 'B' using 'ECSS_SMP_2025'\nsource 'smdl'\ndependency 'A'\n");
  auto client = fx.add("file:///b/smdl/client.xsmpcat",
                       "catalogue Client\nnamespace client { struct S { field "
                       "foundation.Payload payload } }\n");
  fx.build({projectA, mirror, projectB, client});
  auto& manager = *fx.ws.shared().projectManager;
  ASSERT_EQ(manager.getProject(*mirror),
            cursiv::ast_cast<xsmp::Project>(projectA->parseResult.value));
  ASSERT_NE(firstField(client), nullptr);
  ASSERT_TRUE(firstField(client)->type);
  EXPECT_EQ(firstField(client)->type.get()->name, "Payload");
  EXPECT_TRUE(manager.getVisibleDocumentIds(*client)->contains(mirror->id));
  EXPECT_FALSE(manager.getVisibleDocumentIds(*mirror)->contains(client->id));
}

TEST(ProjectManager,
     InnermostProjectOwnsDocumentsAndMirrorsRegardlessOfInsertionOrder) {
  for (const auto outerFirst : {true, false}) {
    SCOPED_TRACE(outerFirst);
    Fixture fx;
    Doc outer, inner;
    const auto outerText =
        "project 'outer' using 'ECSS_SMP_2025'\nsource 'nested/src'\n";
    const auto innerText =
        "project 'inner' using 'ECSS_SMP_2025'\nsource '.'\n";
    if (outerFirst) {
      outer = fx.add("file:///workspace/xsmp.project", outerText);
      inner = fx.add("file:///workspace/nested/xsmp.project", innerText);
    } else {
      inner = fx.add("file:///workspace/nested/xsmp.project", innerText);
      outer = fx.add("file:///workspace/xsmp.project", outerText);
    }
    auto model = fx.add("file:///workspace/nested/src/model.xsmpcat",
                        "catalogue model\n");
    auto mirror = fx.add("xsmp-smp:///workspace/nested/src/imported.smpcat",
                         "catalogue imported\n");
    fx.build({outer, inner, model, mirror});
    auto& manager = *fx.ws.shared().projectManager;
    ASSERT_NE(manager.getProject(*model), nullptr);
    EXPECT_EQ(manager.getProject(*model)->name, "inner");
    ASSERT_NE(manager.getProject(*mirror), nullptr);
    EXPECT_EQ(manager.getProject(*mirror)->name, "inner");
  }
}

TEST(ProjectManager, ContributionPayloadsAreLocalToTheActivatingProject) {
  Fixture fx;
  auto& shared = fx.ws.shared<xsmp::XsmpSharedServices>();
  const std::string payloadUri = "xsmp:/contributions/custom/payload.xsmpcat";
  shared.contributionPayloadOwners[payloadUri] = "custom";
  shared.contributionAliases["legacy-custom"] = "custom";
  auto descriptor =
      fx.add("xsmp:/contributions/custom.xsmpprofile", "profile 'custom'");
  auto payload = fx.add(
      payloadUri,
      "catalogue CustomPayload namespace custom { public struct Added {} }");
  auto a = fx.add("file:///active/xsmp.project",
                  "project 'Active' source 'src' profile 'custom'");
  auto b = fx.add("file:///dependent/xsmp.project",
                  "project 'Dependent' source 'src' dependency 'Active'");
  auto c = fx.add("file:///alias/xsmp.project",
                  "project 'Alias' source 'src' profile 'legacy-custom'");
  const std::string text =
      "catalogue User namespace n { struct S { field custom.Added value } }";
  auto active = fx.add("file:///active/src/a.xsmpcat", text);
  auto dependent = fx.add("file:///dependent/src/b.xsmpcat", text);
  auto alias = fx.add("file:///alias/src/c.xsmpcat", text);
  auto standalone = fx.add("file:///standalone.xsmpcat", text);
  fx.build(
      {descriptor, payload, a, b, c, active, dependent, alias, standalone});
  ASSERT_NE(firstField(active), nullptr);
  EXPECT_NE(firstField(active)->type.get(), nullptr);
  EXPECT_NE(firstField(alias)->type.get(), nullptr);
  EXPECT_EQ(firstField(dependent)->type.get(), nullptr);
  EXPECT_EQ(firstField(standalone)->type.get(), nullptr);
  EXPECT_TRUE(shared.projectManager->getVisibleDocumentIds(*active)->contains(
      payload->id));
  EXPECT_FALSE(shared.projectManager->getVisibleDocumentIds(*dependent)
                   ->contains(payload->id));
}

TEST(ProjectManager, ActivatingAContributionRelinksExistingProjectReferences) {
  Fixture fx;
  auto project = fx.add("file:///late/xsmp.project",
                        "project 'Late' source 'src' tool 'new-tool'");
  fx.build({project});
  const auto* root =
      cursiv::ast_cast<xsmp::Project>(project->parseResult.value);
  ASSERT_NE(root, nullptr);
  const auto* reference =
      cursiv::ast_cast<xsmp::ToolReference>(root->elements.back());
  ASSERT_NE(reference, nullptr);
  EXPECT_EQ(reference->tool.get(), nullptr);
  auto descriptor =
      fx.add("xsmp:/contributions/new-tool.xsmptool", "tool 'new-tool'");
  fx.ws.shared<xsmp::XsmpSharedServices>()
      .projectManager->contributionsChanged();
  fx.build({project, descriptor});
  EXPECT_EQ(reference->tool.get(),
            cursiv::ast_cast<xsmp::Tool>(descriptor->parseResult.value));
  ASSERT_NE(reference->tool.get(), nullptr);
  EXPECT_EQ(reference->tool.get()->name, "new-tool");
}

TEST(ProjectManager,
     SharedPayloadIsVisibleToAnyContributorAndSnapshotsKeepTheUnion) {
  Fixture fx;
  auto& shared = fx.ws.shared<xsmp::XsmpSharedServices>();
  const std::string payloadUri = "xsmp:/contributions/shared/payload.xsmpcat";
  shared.contributionPayloadOwners[payloadUri] = "first";
  shared.contributionPayloadContributors[payloadUri] = {"first", "second"};
  const auto first = fx.add("xsmp:/first.xsmptool", "tool 'first'");
  const auto second = fx.add("xsmp:/second.xsmptool", "tool 'second'");
  const auto payload = fx.add(
      payloadUri,
      "catalogue SharedPayload namespace shared { public struct Payload {} }");
  const auto a = fx.add("file:///first/xsmp.project",
                        "project 'First' source 'src' tool 'first'");
  const auto b = fx.add("file:///second/xsmp.project",
                        "project 'Second' source 'src' tool 'second'");
  const auto c =
      fx.add("file:///both/xsmp.project",
             "project 'Both' source 'src' tool 'first' tool 'second'");
  const auto d = fx.add("file:///dependent/xsmp.project",
                        "project 'Dependent' source 'src' dependency 'First'");
  const std::string text =
      "catalogue User namespace n { struct S { field shared.Payload value } }";
  const auto fromFirst = fx.add("file:///first/src/a.xsmpcat", text);
  const auto fromSecond = fx.add("file:///second/src/b.xsmpcat", text);
  const auto fromBoth = fx.add("file:///both/src/c.xsmpcat", text);
  const auto dependent = fx.add("file:///dependent/src/d.xsmpcat", text);
  const auto standalone = fx.add("file:///standalone.xsmpcat", text);
  fx.build({first, second, payload, a, b, c, d, fromFirst, fromSecond, fromBoth,
            dependent, standalone});
  for (const auto& document : {fromFirst, fromSecond, fromBoth}) {
    ASSERT_NE(firstField(document), nullptr);
    EXPECT_NE(firstField(document)->type.get(), nullptr) << document->uri;
    EXPECT_TRUE(
        shared.projectManager->getVisibleDocumentIds(*document)->contains(
            payload->id));
  }
  for (const auto& document : {dependent, standalone}) {
    ASSERT_NE(firstField(document), nullptr);
    EXPECT_EQ(firstField(document)->type.get(), nullptr) << document->uri;
    EXPECT_FALSE(
        shared.projectManager->getVisibleDocumentIds(*document)->contains(
            payload->id));
  }
}

namespace {

// A workspace that tells which documents the last edit validated.
struct Edits : Fixture, cursiv::DocumentPhaseListener {
  std::mutex mutex;
  std::set<std::string> validated;

  Edits() {
    auto& shared = ws.shared<xsmp::XsmpSharedServices>();
    shared.documentBuilder->addDocumentPhaseListener(*this);
    shared.serviceRegistry->registerServices(
        xsmp::createXsmpcfgServices(shared));
  }

  ~Edits() override {
    ws.shared().documentBuilder->removeDocumentPhaseListener(*this);
  }

  void onDocumentPhase(cursiv::Document& document,
                       cursiv::DocumentState reached,
                       const cursiv::CancellationToken&) override {
    const std::scoped_lock lock(mutex);
    if (reached == cursiv::DocumentState::Validated)
      validated.insert(document.uri);
  }

  void edit(const Doc& document, std::string text) {
    validated.clear();
    xsmp::parse(ws, document->textDocument().languageId(), std::move(text),
                document->uri);
  }
};

}  // namespace

// Editing inside a document, with its exports unchanged, revalidates only those
// that read it through references, at any depth.
TEST(ProjectManager, AnEditReachesOnlyTheReadersOfTheDocument) {
  Edits fx;
  auto project = fx.add("file:///e/xsmp.project", "project 'E' source 'src'\n");
  auto base = fx.add("file:///e/src/base.xsmpcat",
                     "catalogue base namespace b { struct S {} }\n");
  auto middle =
      fx.add("file:///e/src/middle.xsmpcat",
             "catalogue middle namespace m { struct T { field b.S s } }\n");
  auto top = fx.add("file:///e/src/top.xsmpcat",
                    "catalogue top namespace t { struct U { field m.T s } }\n");
  auto unrelated = fx.add("file:///e/src/other.xsmpcat",
                          "catalogue other namespace o { struct V {} }\n");
  auto config = fx.add("file:///e/src/k.xsmpcfg", "configuration K\n");
  fx.build({project, base, middle, top, unrelated, config}, /*validate=*/true);
  using Uris = std::set<std::string>;

  // The built-ins were loaded with no validation, which this first update ends.
  fx.edit(top, "catalogue top namespace t { struct U { field m.T s } } //\n");
  fx.edit(unrelated,
          "catalogue other namespace o { struct V {} } // a comment\n");
  EXPECT_EQ(fx.validated, Uris{unrelated->uri});

  fx.edit(unrelated,
          "catalogue other namespace o { struct V { field Smp.Int32 x } }\n");
  EXPECT_EQ(fx.validated, Uris{unrelated->uri});

  fx.edit(config, "configuration K // a comment\n");
  EXPECT_EQ(fx.validated, Uris{config->uri});

  fx.edit(base,
          "catalogue base namespace b { struct S { field Smp.Int32 x } }\n");
  EXPECT_EQ(fx.validated, (Uris{base->uri, middle->uri, top->uri}));

  // What others can name moved: whoever sees it may be concerned.
  fx.edit(unrelated, "catalogue other namespace o { struct W {} }\n");
  EXPECT_TRUE(fx.validated.contains(base->uri));
  EXPECT_TRUE(fx.validated.contains(config->uri));
}

// A check that reads another document without a reference stays fresh on both
// sides, when the duplicate appears and when it goes.
TEST(ProjectManager, DuplicatedTypeNamesStayFreshAcrossEdits) {
  Edits fx;
  auto project = fx.add("file:///d/xsmp.project", "project 'D' source 'src'\n");
  auto first = fx.add("file:///d/src/a.xsmpcat",
                      "catalogue a namespace n { struct S {} }\n");
  auto second = fx.add("file:///d/src/b.xsmpcat",
                       "catalogue b namespace n { struct T {} }\n");
  fx.build({project, first, second}, /*validate=*/true);
  ASSERT_FALSE(hasError(first, "Duplicated Type name."));

  fx.edit(second, "catalogue b namespace n { struct S {} }\n");
  EXPECT_TRUE(hasError(first, "Duplicated Type name."));
  EXPECT_TRUE(hasError(second, "Duplicated Type name."));

  fx.edit(second, "catalogue b namespace n { struct T {} }\n");
  EXPECT_FALSE(hasError(first, "Duplicated Type name."));
  EXPECT_FALSE(hasError(second, "Duplicated Type name."));
}

TEST(ProjectManager, DuplicatedDocumentAndServiceNamesStayFreshAcrossEdits) {
  Edits fx;
  auto project = fx.add("file:///d/xsmp.project", "project 'D' source 'src'\n");
  auto first = fx.add("file:///d/src/a.xsmpcat",
                      "catalogue a namespace n { service S {} }\n");
  auto second = fx.add("file:///d/src/b.xsmpcat",
                       "catalogue b namespace o { service T {} }\n");
  fx.build({project, first, second}, /*validate=*/true);
  ASSERT_FALSE(hasError(first, "Duplicated Document name."));
  ASSERT_FALSE(hasError(first, "Duplicated Service name."));

  fx.edit(second, "catalogue a namespace o { service T {} }\n");
  EXPECT_TRUE(hasError(first, "Duplicated Document name."));
  fx.edit(second, "catalogue b namespace o { service S {} }\n");
  EXPECT_FALSE(hasError(first, "Duplicated Document name."));
  EXPECT_TRUE(hasError(first, "Duplicated Service name."));
  fx.edit(second, "catalogue b namespace o { service T {} }\n");
  EXPECT_FALSE(hasError(first, "Duplicated Service name."));
}

// A `@uuid` lives in a comment and is not exported.
TEST(ProjectManager, DuplicatedUuidsStayFreshAcrossEdits) {
  constexpr std::string_view uuid = "aaaaaaaa-0000-1111-1111-000000000001";
  const auto catalogue = [](std::string_view name, std::string_view id) {
    return "catalogue " + std::string(name) + " namespace " + std::string(name)
           + " { /** @uuid " + std::string(id) + " */ struct S {} }\n";
  };
  Edits fx;
  auto project = fx.add("file:///d/xsmp.project", "project 'D' source 'src'\n");
  auto first = fx.add("file:///d/src/a.xsmpcat", catalogue("a", uuid));
  auto second = fx.add("file:///d/src/b.xsmpcat", catalogue("b", uuid));
  fx.build({project, first, second}, /*validate=*/true);
  ASSERT_TRUE(hasError(first, "Duplicated UUID."));
  ASSERT_TRUE(hasError(second, "Duplicated UUID."));

  fx.edit(second, catalogue("b", "bbbbbbbb-0000-1111-1111-000000000001"));
  EXPECT_FALSE(hasError(first, "Duplicated UUID."));
  EXPECT_FALSE(hasError(second, "Duplicated UUID."));

  fx.edit(second, catalogue("b", uuid));
  EXPECT_TRUE(hasError(first, "Duplicated UUID."));
}

// Every catalogue of a cycle reads the others through references.
TEST(ProjectManager, BreakingACycleInOneCatalogueClearsItOnTheOthers) {
  Edits fx;
  auto project = fx.add("file:///c/xsmp.project", "project 'C' source 'src'\n");
  auto a = fx.add("file:///c/src/a.xsmpcat",
                  "catalogue a namespace a { struct A { field b.B x } }\n");
  auto b = fx.add("file:///c/src/b.xsmpcat",
                  "catalogue b namespace b { struct B { field c.C x } }\n");
  auto c = fx.add("file:///c/src/c.xsmpcat",
                  "catalogue c namespace c { struct C { field a.A x } }\n");
  fx.build({project, a, b, c}, /*validate=*/true);
  ASSERT_TRUE(hasError(a, "circular dependencies"));

  fx.edit(c, "catalogue c namespace c { struct C {} }\n");
  EXPECT_FALSE(hasError(a, "circular dependencies"));
  EXPECT_FALSE(hasError(b, "circular dependencies"));

  fx.edit(c, "catalogue c namespace c { struct C { field a.A x } }\n");
  EXPECT_TRUE(hasError(a, "circular dependencies"));
}

// A nested project owns its sources: what it declares is visible through a
// dependency, even from a project whose `source` folder holds it.
TEST(ProjectManager, NestedProjectIsVisibleOnlyThroughADependency) {
  for (const auto depends : {false, true}) {
    SCOPED_TRACE(depends);
    Fixture fx;
    auto outer = fx.add("file:///o/xsmp.project",
                        std::string("project 'outer' source '.'\n")
                            + (depends ? "dependency 'inner'\n" : ""));
    auto inner = fx.add("file:///o/inner/xsmp.project",
                        "project 'inner' source 'src'\n");
    auto type = fx.add("file:///o/inner/src/t.xsmpcat",
                       "catalogue t namespace n { primitive P }\n");
    auto user =
        fx.add("file:///o/src/u.xsmpcat",
               "catalogue u namespace n { struct S { field n.P x } }\n");
    fx.build({outer, inner, type, user}, /*validate=*/true);
    auto& manager = *fx.ws.shared().projectManager;
    EXPECT_EQ(manager.getVisibleDocumentIds(*user)->contains(type->id),
              depends);
    ASSERT_NE(firstField(user), nullptr);
    EXPECT_EQ(static_cast<bool>(firstField(user)->type), depends);
    EXPECT_EQ(hasError(user, "add it as a dependency"), !depends);
    EXPECT_EQ(manager.getVisibleDocumentIds(*user),
              manager.getVisibleDocumentIds(*outer))
        << "the documents of a project share one set";
  }
}

// A built-in reads only built-ins: an edit elsewhere leaves it standing. What a
// contribution adds is validated like any other document.
TEST(ProjectManager, EditsLeaveBuiltInsStandingAndPayloadsValidated) {
  Edits fx;
  auto& shared = fx.ws.shared<xsmp::XsmpSharedServices>();
  const std::string payloadUri = "xsmp:/contributions/custom/payload.xsmpcat";
  shared.contributionPayloadOwners[payloadUri] = "custom";
  auto payload = fx.add(
      payloadUri,
      "catalogue Payload namespace custom { struct S { field Missing x } }");
  auto project = fx.add("file:///p/xsmp.project", "project 'P' source 'src'\n");
  auto user = fx.add("file:///p/src/u.xsmpcat", "catalogue u namespace n {}\n");
  std::vector<Doc> all{payload, project, user};
  for (const auto source : xsmp::builtinSources())
    all.push_back(shared.documents->getDocument(source.uri));
  fx.build(all, /*validate=*/true);
  EXPECT_FALSE(payload->diagnostics.empty());

  const auto noBuiltin = [&](std::string_view edited) {
    for (const auto source : xsmp::builtinSources())
      EXPECT_FALSE(fx.validated.contains(std::string(source.uri)))
          << source.uri << " after an edit of " << edited;
  };
  fx.edit(user, "catalogue u namespace n { struct Added {} }\n");
  noBuiltin("a catalogue");
  fx.edit(project, "project 'P' using 'ECSS_SMP_2025' source 'src'\n");
  noBuiltin("a project");
  EXPECT_FALSE(payload->diagnostics.empty());
}

namespace {

std::string uuidNumber(unsigned id) {
  return "af000000-0000-4000-8000-00000000000" + std::to_string(id);
}

std::string projectNamed(std::string_view name, std::string_view more = {}) {
  return "project '" + std::string(name)
         + "' using 'ECSS_SMP_2025'\nsource 'src'\n" + std::string(more);
}

// A catalogue with one public type, in a namespace all of them share.
std::string catalogueOf(std::string_view name, unsigned id,
                        std::string_view declaration) {
  return "catalogue " + std::string(name) + "\nnamespace types {\n/** @uuid "
         + uuidNumber(id) + " */ public " + std::string(declaration) + "\n}\n";
}

bool hasAnyError(const Doc& document) {
  return std::ranges::any_of(document->diagnostics, [](const auto& diagnostic) {
    return diagnostic.severity == cursiv::DiagnosticSeverity::Error;
  });
}

const cursiv::Diagnostic* findDiagnostic(const Doc& document,
                                         std::string_view message) {
  for (const auto& diagnostic : document->diagnostics)
    if (diagnostic.message == message) return &diagnostic;
  return nullptr;
}

std::vector<const xsmp::Parameter*> parametersOf(const Doc& document) {
  std::vector<const xsmp::Parameter*> parameters;
  for (const auto* node : xsmp::descendantsOf(*document->parseResult.value))
    if (const auto* parameter = cursiv::ast_cast<const xsmp::Parameter>(node))
      parameters.push_back(parameter);
  return parameters;
}

}  // namespace

// A value read through a constant follows it, at any depth, and an edit that
// changes no export leaves the other documents alone.
TEST(ProjectManager, AnEditReachesWhatReadsItsValuesAtAnyDepth) {
  const auto limits =
      catalogueOf("A", 1, "struct Limits { public constant Int32 Max = 10 }");
  Edits fx;
  auto app = fx.add("file:///w/app/xsmp.project", projectNamed("app"));
  auto a = fx.add("file:///w/app/src/a.xsmpcat", limits);
  auto b = fx.add(
      "file:///w/app/src/b.xsmpcat",
      catalogueOf("B", 2,
                  "integer Value extends Int32 in 0 ... types.Limits.Max"));
  auto c = fx.add(
      "file:///w/app/src/c.xsmpcat",
      catalogueOf("C", 3, "struct Consumer { field types.Value value = 8 }"));
  auto unrelated =
      fx.add("file:///w/app/src/unrelated.xsmpcat",
             catalogueOf("Unrelated", 4,
                         "struct Unrelated { field Int32 value }"));
  fx.build({app, a, b, c, unrelated}, /*validate=*/true);
  ASSERT_FALSE(hasAnyError(c));
  // The built-ins were loaded with no validation, which this first update ends.
  fx.edit(unrelated, catalogueOf("Unrelated", 4,
                                 "struct Unrelated { field Int32 value } //"));
  using Uris = std::set<std::string>;

  fx.edit(a, "// A comment\n" + limits);
  EXPECT_EQ(fx.validated, (Uris{a->uri, b->uri, c->uri}));
  EXPECT_FALSE(hasAnyError(c));

  auto lower = limits;
  lower.replace(lower.find("= 10"), 4, "= 5");
  fx.edit(a, lower);
  EXPECT_EQ(fx.validated, (Uris{a->uri, b->uri, c->uri}));
  EXPECT_TRUE(hasAnyError(c));

  fx.edit(a, limits);
  EXPECT_FALSE(hasAnyError(c));
}

// An attribute may read its value in another document.
TEST(ProjectManager, AnAttributeFollowsTheConstantItReads) {
  const auto flags = [](std::string_view value) {
    return catalogueOf("Flags", 1,
                       "struct Flags { public constant Bool Enabled = "
                           + std::string(value) + " }");
  };
  constexpr std::string_view message =
      "A pointer parameter default shall be nullptr.";
  Edits fx;
  auto app = fx.add("file:///w/app/xsmp.project", projectNamed("app"));
  auto source = fx.add("file:///w/app/src/flags.xsmpcat", flags("false"));
  auto consumer = fx.add(
      "file:///w/app/src/consumer.xsmpcat",
      catalogueOf("Consumer", 2,
                  "model Consumer { @Static(types.Flags.Enabled) field Int32 "
                  "counter def void compute(@ByPointer(types.Flags.Enabled) "
                  "in Int32 value = 7) }"));
  fx.build({app, source, consumer}, /*validate=*/true);
  ASSERT_EQ(findDiagnostic(consumer, message), nullptr);
  const auto* parameter = parametersOf(consumer).at(0);
  EXPECT_FALSE(xsmp::isByPointer(*parameter));

  fx.edit(source, flags("true"));
  EXPECT_TRUE(xsmp::isByPointer(*parameter));
  EXPECT_NE(findDiagnostic(consumer, message), nullptr);

  fx.edit(source, flags("false"));
  EXPECT_FALSE(xsmp::isByPointer(*parameter));
  EXPECT_EQ(findDiagnostic(consumer, message), nullptr);
}

// How a parameter is passed by default depends on the kind of its type.
TEST(ProjectManager, APassingModeFollowsTheKindOfTheParameterType) {
  Edits fx;
  auto app = fx.add("file:///w/app/xsmp.project", projectNamed("app"));
  auto types = fx.add("file:///w/app/src/types.xsmpcat",
                      catalogueOf("Types", 1, "struct Item {}"));
  auto consumer =
      fx.add("file:///w/app/src/consumer.xsmpcat",
             catalogueOf("Consumer", 2,
                         "model Consumer { def void compute(in types.Item "
                         "value) }"));
  fx.build({app, types, consumer}, /*validate=*/true);
  const auto* parameter = parametersOf(consumer).at(0);
  EXPECT_FALSE(xsmp::isConst(*parameter));
  EXPECT_FALSE(xsmp::isByReference(*parameter));

  fx.edit(types, catalogueOf("Types", 1, "interface Item {}"));
  EXPECT_TRUE(xsmp::isConst(*parameter));
  EXPECT_TRUE(xsmp::isByReference(*parameter));
  EXPECT_FALSE(hasAnyError(consumer));
}

// A `@uuid` is unique across the workspace, whatever each project sees, and a
// diagnostic that points into an edited document follows it.
TEST(ProjectManager, DuplicatedUuidsReachOtherProjectsAndTheirLocationsFollow) {
  Edits fx;
  auto first = fx.add("file:///w/a/xsmp.project", projectNamed("a"));
  auto second = fx.add("file:///w/b/xsmp.project", projectNamed("b"));
  const auto text = catalogueOf("A", 1, "struct First {}");
  auto a = fx.add("file:///w/a/src/a.xsmpcat", text);
  auto b = fx.add("file:///w/b/src/b.xsmpcat",
                  catalogueOf("B", 2, "struct Second {}"));
  fx.build({first, second, a, b}, /*validate=*/true);
  constexpr std::string_view message = "Duplicated UUID.";
  ASSERT_EQ(findDiagnostic(b, message), nullptr);

  const auto sameUuid = catalogueOf("A", 2, "struct First {}");
  fx.edit(a, sameUuid);
  EXPECT_NE(findDiagnostic(a, message), nullptr);
  const auto* duplicate = findDiagnostic(b, message);
  ASSERT_NE(duplicate, nullptr);
  ASSERT_EQ(duplicate->relatedInformation.size(), 1U);
  const auto begin = duplicate->relatedInformation[0].range.begin;

  fx.edit(a, "\n\n" + sameUuid);
  duplicate = findDiagnostic(b, message);
  ASSERT_NE(duplicate, nullptr);
  ASSERT_EQ(duplicate->relatedInformation.size(), 1U);
  EXPECT_EQ(duplicate->relatedInformation[0].range.begin, begin + 2);

  fx.edit(a, text);
  EXPECT_EQ(findDiagnostic(b, message), nullptr);
}

// A cancelled update leaves its work to the next one, whatever it had reached.
TEST(ProjectManager, AResumedUpdateFinishesWhatACancelledOneLeft) {
  struct Cancel : cursiv::BuildPhaseListener {
    std::stop_source source;
    cursiv::DocumentState at;
    explicit Cancel(cursiv::DocumentState state) : at(state) {}
    void onBuildPhase(cursiv::DocumentState reached,
                      std::span<cursiv::Document* const>,
                      const cursiv::CancellationToken&) override {
      if (reached == at) source.request_stop();
    }
  };
  for (const auto state : {cursiv::DocumentState::IndexedContent,
                           cursiv::DocumentState::ComputedScopes}) {
    SCOPED_TRACE(static_cast<int>(state));
    Edits fx;
    auto& shared = fx.ws.shared<xsmp::XsmpSharedServices>();
    auto app = fx.add("file:///w/app/xsmp.project", projectNamed("app"));
    auto other = fx.add("file:///w/other/xsmp.project", projectNamed("other"));
    auto a = fx.add("file:///w/app/src/a.xsmpcat",
                    catalogueOf("A", 1, "struct First {}"));
    auto b = fx.add("file:///w/app/src/b.xsmpcat",
                    catalogueOf("B", 2, "struct Second {}"));
    auto c = fx.add("file:///w/other/src/c.xsmpcat",
                    catalogueOf("C", 3, "struct Third {}"));
    fx.build({app, other, a, b, c}, /*validate=*/true);

    Cancel cancel(state);
    shared.documentBuilder->addBuildPhaseListener(cancel);
    shared.documentFactory->attachTextDocument(
        *a, cursiv::TextDocument::create(
                a->uri, "xsmpcat", a->textDocument().version() + 1,
                catalogueOf("A", 3, "struct Second {}")));
    a->isOpen = true;
    const std::string_view changed[] = {a->uri};
    EXPECT_ANY_THROW(
        shared.documentBuilder->update(changed, {}, cancel.source.get_token()));
    shared.documentBuilder->removeBuildPhaseListener(cancel);

    shared.documentBuilder->update({}, {});
    EXPECT_NE(findDiagnostic(a, "Duplicated Type name."), nullptr);
    EXPECT_NE(findDiagnostic(b, "Duplicated Type name."), nullptr);
    EXPECT_NE(findDiagnostic(c, "Duplicated UUID."), nullptr);
    EXPECT_EQ(b->state.load(), cursiv::DocumentState::Validated);
    EXPECT_EQ(c->state.load(), cursiv::DocumentState::Validated);
  }
}

// Which of two types of the same name answers does not depend on what was
// edited last.
TEST(ProjectManager, AnEditKeepsWhichOfTwoSameNamedTypesAnswers) {
  auto shadowing = catalogueOf("A", 1, "struct Shared {}");
  shadowing.replace(shadowing.find("namespace types"), 15,
                    "namespace Smp::types");
  const auto other = catalogueOf("B", 2, "struct Shared {}");
  Edits fx;
  auto app = fx.add("file:///w/app/xsmp.project", projectNamed("app"));
  auto a = fx.add("file:///w/app/src/a.xsmpcat", shadowing);
  auto b = fx.add("file:///w/app/src/b.xsmpcat", other);
  auto c = fx.add(
      "file:///w/app/src/c.xsmpcat",
      catalogueOf("C", 3, "struct Consumer { field types.Shared value }"));
  fx.build({app, a, b, c}, /*validate=*/true);
  ASSERT_NE(firstField(c)->type.get(), nullptr);
  const auto winner = firstField(c)->type.get()->getDocument()->uri;

  fx.edit(a, "// A comment\n" + shadowing);
  EXPECT_EQ(firstField(c)->type.get()->getDocument()->uri, winner);
  fx.edit(b, "// A comment\n" + other);
  EXPECT_EQ(firstField(c)->type.get()->getDocument()->uri, winner);
}

// A project document that its own `source` does not hold, a document of no
// project and a built-in see only what is theirs.
TEST(ProjectManager, OnlyTheDocumentsOfAProjectShareWhatTheySee) {
  Fixture fx;
  auto app = fx.add("file:///w/app/xsmp.project", projectNamed("app"));
  auto a = fx.add("file:///w/app/src/a.xsmpcat",
                  catalogueOf("A", 1, "struct First {}"));
  auto b = fx.add("file:///w/app/src/b.xsmpcat",
                  catalogueOf("B", 2, "struct Second {}"));
  auto one = fx.add("file:///w/one.xsmpcat",
                    catalogueOf("One", 3, "struct StandaloneOne {}"));
  auto two = fx.add("file:///w/two.xsmpcat",
                    catalogueOf("Two", 4, "struct StandaloneTwo {}"));
  fx.build({app, a, b, one, two}, /*validate=*/true);
  auto& manager = *fx.ws.shared().projectManager;
  const auto seen = [&](const Doc& document) {
    return manager.getVisibleDocumentIds(*document);
  };

  EXPECT_EQ(manager.getProject(*app), nullptr);
  EXPECT_EQ(seen(a), seen(b));
  EXPECT_TRUE(seen(a)->contains(a->id));
  EXPECT_TRUE(seen(a)->contains(b->id));
  EXPECT_NE(seen(app), seen(a));
  EXPECT_FALSE(seen(app)->contains(a->id));

  EXPECT_NE(seen(one), seen(two));
  EXPECT_TRUE(seen(one)->contains(one->id));
  EXPECT_FALSE(seen(one)->contains(two->id));
  EXPECT_FALSE(seen(a)->contains(one->id));

  auto& shared = fx.ws.shared<xsmp::XsmpSharedServices>();
  for (const auto source : xsmp::builtinSources()) {
    auto builtin = shared.documents->getDocument(source.uri);
    ASSERT_NE(builtin, nullptr);
    EXPECT_TRUE(seen(builtin)->contains(builtin->id));
    EXPECT_FALSE(seen(builtin)->contains(a->id));
  }
}

// A document that arrives is seen by all at once, and so is one that leaves.
TEST(ProjectManager, ACatalogueThatComesAndGoesRefreshesTheOthers) {
  Edits fx;
  auto app = fx.add("file:///w/app/xsmp.project", projectNamed("app"));
  auto a = fx.add("file:///w/app/src/a.xsmpcat",
                  catalogueOf("A", 1, "struct Shared {}"));
  auto b = fx.add(
      "file:///w/app/src/b.xsmpcat",
      catalogueOf("B", 2, "struct Consumer { field types.Shared value }"));
  fx.build({app, a, b}, /*validate=*/true);
  ASSERT_FALSE(hasAnyError(a));
  auto& builder = *fx.ws.shared().documentBuilder;
  auto& manager = *fx.ws.shared().projectManager;

  constexpr std::string_view added = "file:///w/app/src/new.xsmpcat";
  auto fresh = fx.add(std::string(added),
                      catalogueOf("New", 3, "struct Shared {}"));
  builder.update(std::span(&added, 1), {});
  EXPECT_NE(findDiagnostic(a, "Duplicated Type name."), nullptr);
  EXPECT_NE(findDiagnostic(fresh, "Duplicated Type name."), nullptr);
  EXPECT_TRUE(manager.getVisibleDocumentIds(*a)->contains(fresh->id));
  EXPECT_EQ(manager.getVisibleDocumentIds(*a),
            manager.getVisibleDocumentIds(*b));

  fx.edit(fresh, catalogueOf("New", 3, "struct Renamed {}"));
  EXPECT_FALSE(hasAnyError(a));
  EXPECT_FALSE(hasAnyError(fresh));
  ASSERT_NE(firstField(b)->type.get(), nullptr);
  EXPECT_EQ(firstField(b)->type.get()->getDocument()->uri, a->uri);

  fx.edit(fresh, catalogueOf("New", 3, "struct Shared {}"));
  EXPECT_NE(findDiagnostic(a, "Duplicated Type name."), nullptr);
  const auto freshId = fresh->id;
  builder.update({}, std::span(&added, 1));
  EXPECT_FALSE(hasAnyError(a));
  EXPECT_FALSE(hasAnyError(b));
  EXPECT_FALSE(manager.getVisibleDocumentIds(*a)->contains(freshId));
}

// A reference that did not resolve looks again when the exports change, and one
// that did reports its target when it goes.
TEST(ProjectManager, AMissingReferenceFollowsTheExports) {
  const auto text = catalogueOf("A", 1, "struct Existing {}");
  Edits fx;
  auto app = fx.add("file:///w/app/xsmp.project", projectNamed("app"));
  auto a = fx.add("file:///w/app/src/a.xsmpcat", text);
  auto b = fx.add(
      "file:///w/app/src/b.xsmpcat",
      catalogueOf("B", 2, "struct Consumer { field types.Missing value }"));
  fx.build({app, a, b}, /*validate=*/true);
  ASSERT_TRUE(hasAnyError(b));

  fx.edit(a, catalogueOf("A", 1, "struct Missing {}"));
  EXPECT_FALSE(hasAnyError(b));
  fx.edit(a, text);
  EXPECT_TRUE(hasAnyError(b));
}

// The dependency of a project may be written before that project exists.
TEST(ProjectManager, ADependencyOnAProjectAddedLaterResolves) {
  Edits fx;
  auto foundation = fx.add("file:///w/foundation/xsmp.project",
                           projectNamed("foundation", "dependency 'mission'\n"));
  fx.build({foundation}, /*validate=*/true);
  auto& manager = *fx.ws.shared().projectManager;
  ASSERT_EQ(manager.getDependencies(*manager.getProjectByName("foundation"))
                .size(),
            1U);

  constexpr std::string_view added = "file:///w/mission/xsmp.project";
  auto mission = fx.add(std::string(added), projectNamed("mission"));
  fx.ws.shared().documentBuilder->update(std::span(&added, 1), {});
  EXPECT_EQ(manager.getDependencies(*manager.getProjectByName("foundation"))
                .size(),
            2U);
}

// A built-in reads only built-ins, even when a catalogue takes its name.
TEST(ProjectManager, ACatalogueNamedLikeABuiltInLeavesTheBuiltInAlone) {
  Edits fx;
  auto& shared = fx.ws.shared<xsmp::XsmpSharedServices>();
  auto app = fx.add("file:///w/app/xsmp.project", projectNamed("app"));
  std::vector<Doc> all{app};
  for (const auto source : xsmp::builtinSources())
    all.push_back(shared.documents->getDocument(source.uri));
  fx.build(all, /*validate=*/true);
  std::unordered_map<std::string, std::size_t> before;
  for (const auto& document : all)
    before[document->uri] = document->diagnostics.size();
  std::string_view name;
  for (const auto& document : all)
    if (const auto* catalogue =
            cursiv::ast_cast<const xsmp::Catalogue>(document->parseResult.value))
      name = catalogue->name;
  ASSERT_FALSE(name.empty());

  constexpr std::string_view added = "file:///w/app/src/user.xsmpcat";
  auto user = fx.add(std::string(added),
                     catalogueOf(name, 1, "struct UserType {}"));
  shared.documentBuilder->update(std::span(&added, 1), {});
  EXPECT_NE(findDiagnostic(user, "Duplicated Document name."), nullptr);
  for (const auto& document : all)
    EXPECT_EQ(document->diagnostics.size(), before[document->uri])
        << document->uri;
}

// Which project owns a document follows its `source` folders and the projects
// that exist.
TEST(ProjectManager, OwnershipFollowsSourcesAndProjects) {
  Edits fx;
  auto app = fx.add("file:///w/app/xsmp.project", projectNamed("app"));
  auto a = fx.add("file:///w/app/src/a.xsmpcat",
                  catalogueOf("A", 1, "struct First {}"));
  auto b = fx.add("file:///w/app/other/b.xsmpcat",
                  catalogueOf("B", 2, "struct Second {}"));
  auto c = fx.add("file:///w/added/src/c.xsmpcat",
                  catalogueOf("C", 3, "struct Third {}"));
  fx.build({app, a, b, c}, /*validate=*/true);
  auto& manager = *fx.ws.shared().projectManager;
  EXPECT_EQ(manager.getProject(*a), manager.getProjectByName("app"));
  EXPECT_EQ(manager.getProject(*b), nullptr);
  EXPECT_EQ(manager.getProject(*c), nullptr);

  fx.edit(app, "project 'app' using 'ECSS_SMP_2025'\nsource 'other'\n");
  EXPECT_EQ(manager.getProject(*a), nullptr);
  EXPECT_EQ(manager.getProject(*b), manager.getProjectByName("app"));
  EXPECT_FALSE(manager.getVisibleDocumentIds(*b)->contains(a->id));

  constexpr std::string_view added = "file:///w/added/xsmp.project";
  auto project = fx.add(std::string(added), projectNamed("added"));
  auto& builder = *fx.ws.shared().documentBuilder;
  builder.update(std::span(&added, 1), {});
  EXPECT_EQ(manager.getProject(*c), manager.getProjectByName("added"));

  builder.update({}, std::span(&added, 1));
  EXPECT_EQ(manager.getProjectByName("added"), nullptr);
  EXPECT_EQ(manager.getProject(*c), nullptr);
}

// The names of documents, services and projects are unique across the
// workspace, whatever each project sees.
TEST(ProjectManager, UniqueNamesReachOtherProjects) {
  Edits fx;
  auto first = fx.add("file:///w/a/xsmp.project", projectNamed("a"));
  auto second = fx.add("file:///w/b/xsmp.project", projectNamed("b"));
  const auto text = std::string("catalogue A namespace n { service S {} }\n");
  auto a = fx.add("file:///w/a/src/a.xsmpcat", text);
  auto b = fx.add("file:///w/b/src/b.xsmpcat",
                  "catalogue B namespace m { service T {} }\n");
  fx.build({first, second, a, b}, /*validate=*/true);
  const auto duplicates = [&] {
    return findDiagnostic(b, "Duplicated Document name.") != nullptr
           || findDiagnostic(b, "Duplicated Service name.") != nullptr;
  };
  ASSERT_FALSE(duplicates());

  fx.edit(a, "catalogue B namespace n { service S {} }\n");
  EXPECT_NE(findDiagnostic(b, "Duplicated Document name."), nullptr);
  fx.edit(a, "catalogue A namespace n { service T {} }\n");
  EXPECT_EQ(findDiagnostic(b, "Duplicated Document name."), nullptr);
  EXPECT_NE(findDiagnostic(b, "Duplicated Service name."), nullptr);
  fx.edit(a, text);
  EXPECT_FALSE(duplicates());

  fx.edit(first, projectNamed("b"));
  EXPECT_NE(findDiagnostic(second, "Duplicated project name"), nullptr);
  fx.edit(first, projectNamed("a"));
  EXPECT_EQ(findDiagnostic(second, "Duplicated project name"), nullptr);
}
