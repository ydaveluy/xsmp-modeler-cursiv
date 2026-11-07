#include <gtest/gtest.h>

#include <filesystem>
#include <xsmp/Contributions.hpp>
#include <xsmp/ProjectCreation.hpp>
#include <xsmp/utils/FileWriter.hpp>

#include "support/Files.hpp"

TEST(ProjectCreation, NamesTheCatalogueFileByTheIdentifierItDeclares) {
  xsmp::ScratchDirectory directory;
  xsmp::createProject({}, {.projectName = "my.lib-core",
                           .projectDir = directory.path / "my.lib-core"});
  const auto text =
      xsmp::readFile(directory.path / "my.lib-core/smdl/my_lib_core.xsmpcat");
  EXPECT_NE(text.find("catalogue my_lib_core"), std::string::npos);
}

TEST(ProjectCreation, AProjectNamedAfterACppKeywordGetsAValidIdentifier) {
  for (const auto& [name, identifier] : {std::pair{"class", "class_"},
                                         {"namespace", "namespace_"},
                                         {"class_", "class_"},
                                         {"Ordinary", "Ordinary"},
                                         {"A-B", "A_B"}})
    EXPECT_EQ(xsmp::projectIdentifier(name), identifier) << name;
}

TEST(ProjectCreation, CreatesMissingParentsButNeverAnExistingFolder) {
  xsmp::ScratchDirectory directory;
  const auto project = directory.path / "not/yet/there/Mission";
  xsmp::createProject({}, {.projectName = "Mission", .projectDir = project});
  EXPECT_TRUE(std::filesystem::is_regular_file(project / "xsmp.project"));
  EXPECT_THROW(xsmp::createProject(
                   {}, {.projectName = "Mission", .projectDir = project}),
               std::invalid_argument);
}
