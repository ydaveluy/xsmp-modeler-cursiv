#include <gtest/gtest.h>

#include <cursiv/services/Diagnostic.hpp>
#include <cursiv/utils/UriUtils.hpp>
#include <cursiv/workspace/Document.hpp>
#include <cursiv/workspace/Documents.hpp>
#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <xsmp/SharedCoreServices.hpp>
#include <xsmp/XsmpprojectCoreModule.hpp>

#include "support/DiagnosticTestSupport.hpp"

namespace {

using Doc = std::shared_ptr<cursiv::Document>;

// Builds, links and validates a set of `xsmp.project` / `.xsmpprofile` /
// `.xsmptool` documents together, so cross references resolve and the semantic
// checks attach their diagnostics to each document.
struct Fixture {
  std::shared_ptr<xsmp::XsmpSharedCoreServices> shared;

  Fixture() : shared(xsmp::createXsmpSharedServices()) {
    shared->serviceRegistry->registerServices(
        xsmp::createXsmpprojectServices(*shared));
  }

  Doc add(std::string uri, std::string text) {
    return shared->documents->createDocument(std::move(uri), std::move(text));
  }

  void build(const std::vector<Doc>& docs) {
    xsmp::buildDocuments(*shared->documentBuilder, docs,
                         cursiv::BuildOptions{});
  }
};

// True if `doc` carries an error diagnostic whose message contains `needle`.
bool hasError(const Doc& doc, std::string_view needle) {
  for (const auto& d : doc->diagnostics) {
    if (d.severity == cursiv::DiagnosticSeverity::Error
        && d.message.find(needle) != std::string::npos) {
      return true;
    }
  }
  return false;
}

// The first error diagnostic whose message contains `needle`, or nullptr.
const cursiv::Diagnostic* findError(const Doc& doc, std::string_view needle) {
  for (const auto& d : doc->diagnostics) {
    if (d.severity == cursiv::DiagnosticSeverity::Error
        && d.message.find(needle) != std::string::npos) {
      return &d;
    }
  }
  return nullptr;
}

}  // namespace

// --- File extension / name -------------------------------------------------

TEST(XsmpprojectValidator, ProfileWrongExtension) {
  Fixture fx;
  // A Profile declaration parsed from a non-`.xsmpprofile` file.
  auto doc = fx.add("file:///p/sample.xsmptool", "profile 'a'\n");
  fx.build({doc});
  EXPECT_TRUE(hasError(doc, "'.xsmpprofile' file extension"));
}

TEST(XsmpprojectValidator, ProfileCorrectExtension) {
  Fixture fx;
  auto doc = fx.add("file:///p/sample.xsmpprofile", "profile 'a'\n");
  fx.build({doc});
  EXPECT_FALSE(hasError(doc, "'.xsmpprofile' file extension"));
}

TEST(XsmpprojectValidator, ToolWrongExtension) {
  Fixture fx;
  auto doc = fx.add("file:///p/sample.xsmpprofile", "tool 't'\n");
  fx.build({doc});
  EXPECT_TRUE(hasError(doc, "'.xsmptool' file extension"));
}

TEST(XsmpprojectValidator, ProjectWrongFileName) {
  Fixture fx;
  // A Project parsed from a file that is not named `xsmp.project` (the entry
  // rule tries Project first, so `project ...` in a `.xsmpprofile` file parses
  // as a Project).
  auto doc = fx.add("file:///p/bad.xsmpprofile", "project 'P'\n");
  fx.build({doc});
  EXPECT_TRUE(hasError(doc, "file name shall be 'xsmp.project'"));
}

// The diagnostic anchors on the `project` keyword itself, not the whole node.
TEST(XsmpprojectValidator, ProjectWrongFileNameAnchorsOnKeyword) {
  Fixture fx;
  const std::string text = "project 'P'\n";
  auto doc = fx.add("file:///p/bad.xsmpprofile", text);
  fx.build({doc});

  auto found = false;
  const auto keywordBegin = text.find("project");
  ASSERT_NE(keywordBegin, std::string::npos);
  for (const auto& diagnostic : doc->diagnostics) {
    if (diagnostic.message.find("file name shall be 'xsmp.project'")
        != std::string::npos) {
      found = true;
      EXPECT_EQ(diagnostic.range.begin, keywordBegin);
      EXPECT_EQ(diagnostic.range.end,
                keywordBegin + std::string_view("project").size());
    }
  }
  EXPECT_TRUE(found);
}

TEST(XsmpprojectValidator, ProjectCorrectFileName) {
  Fixture fx;
  auto doc = fx.add("file:///p/xsmp.project", "project 'P'\n");
  fx.build({doc});
  EXPECT_FALSE(hasError(doc, "file name shall be 'xsmp.project'"));
}

// --- Standard --------------------------------------------------------------

TEST(XsmpprojectValidator, UnknownStandard) {
  Fixture fx;
  auto doc = fx.add("file:///p/xsmp.project", "project 'P' using 'FOO'\n");
  fx.build({doc});
  EXPECT_TRUE(hasError(doc, "Unknown version"));
}

TEST(XsmpprojectValidator, KnownStandard) {
  Fixture fx;
  auto doc =
      fx.add("file:///p/xsmp.project", "project 'P' using 'ECSS_SMP_2025'\n");
  fx.build({doc});
  EXPECT_FALSE(hasError(doc, "Unknown version"));
}

// --- Duplicated project name (global index) --------------------------------

TEST(XsmpprojectValidator, DuplicatedProjectName) {
  Fixture fx;
  auto a = fx.add("file:///a/xsmp.project", "project 'Same'\n");
  auto b = fx.add("file:///b/xsmp.project", "project 'Same'\n");
  fx.build({a, b});

  const auto* diagnostic = findError(a, "Duplicated project name");
  ASSERT_NE(diagnostic, nullptr);
  // relatedInformation points at the OTHER project's declaration (its
  // document).
  ASSERT_EQ(diagnostic->relatedInformation.size(), 1U);
  EXPECT_EQ(diagnostic->relatedInformation[0].uri, "file:///b/xsmp.project");
  EXPECT_EQ(diagnostic->relatedInformation[0].message, "Same");
}

// --- Profile / tool references ---------------------------------------------

TEST(XsmpprojectValidator, ProfileAlreadyDefined) {
  Fixture fx;
  auto pa = fx.add("file:///p/a.xsmpprofile", "profile 'a'\n");
  auto pb = fx.add("file:///p/b.xsmpprofile", "profile 'b'\n");
  auto project = fx.add("file:///p/xsmp.project",
                        "project 'P'\nprofile 'a'\nprofile 'b'\n");
  fx.build({pa, pb, project});
  EXPECT_TRUE(hasError(project, "A profile is already defined"));
}

TEST(XsmpprojectValidator, DuplicatedTool) {
  Fixture fx;
  auto tool = fx.add("file:///p/t.xsmptool", "tool 't'\n");
  auto project =
      fx.add("file:///p/xsmp.project", "project 'P'\ntool 't'\ntool 't'\n");
  fx.build({tool, project});
  EXPECT_TRUE(hasError(project, "Duplicated tool 't'"));
}

// --- Dependencies ----------------------------------------------------------

TEST(XsmpprojectValidator, CyclicDependency) {
  Fixture fx;
  auto a = fx.add("file:///a/xsmp.project", "project 'A'\ndependency 'B'\n");
  auto b = fx.add("file:///b/xsmp.project", "project 'B'\ndependency 'A'\n");
  fx.build({a, b});
  EXPECT_TRUE(hasError(a, "Cyclic dependency detected 'B'"));
}

TEST(XsmpprojectValidator, DuplicatedDependency) {
  Fixture fx;
  auto q = fx.add("file:///q/xsmp.project", "project 'Q'\n");
  auto p = fx.add("file:///p/xsmp.project",
                  "project 'P'\ndependency 'Q'\ndependency 'Q'\n");
  fx.build({q, p});
  EXPECT_TRUE(hasError(p, "Duplicated dependency 'Q'"));
}

TEST(XsmpprojectValidator, DependencyStandardMismatch) {
  Fixture fx;
  auto q =
      fx.add("file:///q/xsmp.project", "project 'Q' using 'ECSS_SMP_2020'\n");
  auto p = fx.add("file:///p/xsmp.project",
                  "project 'P' using 'ECSS_SMP_2025'\ndependency 'Q'\n");
  fx.build({q, p});
  EXPECT_TRUE(hasError(p, "SMP version does not match 'ECSS_SMP_2025'"));
}

// A project that omits `using` (empty `standard`) depending on one that
// explicitly spells out the default (`ECSS_SMP_2020`) is NOT a mismatch: both
// sides resolve to the same effective standard.
TEST(XsmpprojectValidator, DependencyStandardMatchesWhenOneSideOmitsUsing) {
  Fixture fx;
  auto q =
      fx.add("file:///q/xsmp.project", "project 'Q' using 'ECSS_SMP_2020'\n");
  auto p = fx.add("file:///p/xsmp.project", "project 'P'\ndependency 'Q'\n");
  fx.build({q, p});
  EXPECT_FALSE(hasError(p, "SMP version does not match"));
}

// The mismatch message reports the resolved (defaulted) standard, not a raw
// empty string, when the current project itself omits `using`.
TEST(XsmpprojectValidator, DependencyStandardMismatchMessageResolvesDefault) {
  Fixture fx;
  auto q =
      fx.add("file:///q/xsmp.project", "project 'Q' using 'ECSS_SMP_2025'\n");
  auto p = fx.add("file:///p/xsmp.project", "project 'P'\ndependency 'Q'\n");
  fx.build({q, p});
  EXPECT_TRUE(hasError(p, "SMP version does not match 'ECSS_SMP_2020'"));
}

// --- Source path containment -----------------------------------------------

TEST(XsmpprojectValidator, SourcePathNotContained) {
  Fixture fx;
  auto doc =
      fx.add("file:///p/xsmp.project", "project 'P'\nsource '../outside'\n");
  fx.build({doc});
  EXPECT_TRUE(hasError(doc, "is not contained within the project directory"));
}

TEST(XsmpprojectValidator, SourcePathContained) {
  Fixture fx;
  auto doc = fx.add("file:///p/xsmp.project", "project 'P'\nsource 'src'\n");
  fx.build({doc});
  EXPECT_FALSE(hasError(doc, "is not contained within the project directory"));
}

// --- Source path existence ---------------------------------------------------

// A `source` folder that is contained but does not exist on disk is an error.
// The check uses the real file system, as in a CLI or LSP session.
TEST(XsmpprojectValidator, SourcePathDoesNotExist) {
  Fixture fx;
  auto doc = fx.add("file:///p/xsmp.project",
                    "project 'P'\nsource 'no-such-folder'\n");
  fx.build({doc});
  EXPECT_TRUE(hasError(doc, "Source path 'no-such-folder' does not exist"));
}

// A `source` folder that exists on disk (the `smdl/` folder next to the
// `fixtures/core/xsmpproject/xsmp.project` fixture) is accepted. The in-memory
// text of the document is unrelated to the file at that path: only the
// directory matters.
TEST(XsmpprojectValidator, SourcePathExists) {
  Fixture fx;
  auto doc =
      fx.add(cursiv::path_to_file_uri(std::string(XSMP_FIXTURES_DIR)
                                      + "/core/xsmpproject/xsmp.project"),
             "project 'P'\nsource 'smdl'\n");
  fx.build({doc});
  EXPECT_FALSE(hasError(doc, "does not exist"));
}
