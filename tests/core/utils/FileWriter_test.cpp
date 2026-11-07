#include <gtest/gtest.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <system_error>
#include <vector>
#include <xsmp/utils/FileWriter.hpp>
#include <xsmp/utils/Parallel.hpp>

#include "support/Files.hpp"

TEST(FileWriter, FileExistsIsFalseForMissingPath) {
  xsmp::ScratchDirectory dir;
  EXPECT_FALSE(xsmp::fileExists(dir.path / "missing.txt"));
}

TEST(FileWriter, WriteFileIfChangedCreatesParentDirectoriesAndWrites) {
  xsmp::ScratchDirectory dir;
  const auto path = dir.path / "nested" / "deep" / "out.txt";
  EXPECT_TRUE(xsmp::writeFileIfChanged(path, "hello"));
  ASSERT_TRUE(xsmp::fileExists(path));
  EXPECT_EQ(xsmp::readFile(path), "hello");
}

// Regenerating identical content must not touch the file — this is the
// whole point of writeFileIfChanged: the Generation Gap pattern's src-gen/
// tree is always "regenerated", but an unchanged file should not appear
// modified (no spurious mtime churn / incremental-rebuild triggers).
TEST(FileWriter, WriteFileIfChangedSkipsIdenticalContent) {
  xsmp::ScratchDirectory dir;
  const auto path = dir.path / "out.txt";
  ASSERT_TRUE(xsmp::writeFileIfChanged(path, "same"));
  EXPECT_FALSE(xsmp::writeFileIfChanged(path, "same"));
  EXPECT_EQ(xsmp::readFile(path), "same");
}

TEST(FileWriter, WriteFileIfChangedOverwritesDifferentContent) {
  xsmp::ScratchDirectory dir;
  const auto path = dir.path / "out.txt";
  ASSERT_TRUE(xsmp::writeFileIfChanged(path, "old"));
  EXPECT_TRUE(xsmp::writeFileIfChanged(path, "new"));
  EXPECT_EQ(xsmp::readFile(path), "new");
}

TEST(FileWriter, DirectoryAtOutputPathIsAnErrorEvenForEmptyContent) {
  xsmp::ScratchDirectory dir;
  const auto path = dir.path / "__init__.py";
  std::filesystem::create_directory(path);
  try {
    (void)xsmp::writeFileIfChanged(path, "");
    FAIL()
        << "An output directory cannot be reported as an unchanged empty file";
  } catch (const std::runtime_error& error) {
    EXPECT_NE(std::string_view(error.what()).find("Error generating file"),
              std::string_view::npos);
    EXPECT_NE(std::string_view(error.what()).find("__init__.py"),
              std::string_view::npos);
  }
  EXPECT_TRUE(std::filesystem::is_directory(path));
}

TEST(FileWriter, ParentDirectoryCreationFailurePreservesExistingFile) {
  xsmp::ScratchDirectory dir;
  const auto parent = dir.path / "blocked";
  std::ofstream(parent) << "keep this content";
  EXPECT_THROW(xsmp::writeFileIfChanged(parent / "out.txt", "generated"),
               std::runtime_error);
  EXPECT_EQ(xsmp::readFile(parent), "keep this content");
}

TEST(FileWriter, FlushFailureIsReportedInsteadOfSuccessfulGeneration) {
  if (!std::filesystem::exists("/dev/full"))
    GTEST_SKIP() << "No full-device fixture on this platform";
  EXPECT_THROW(xsmp::writeFileIfChanged("/dev/full", "generated"),
               std::runtime_error);
}

TEST(FileWriter, PathTextIsUtf8AndReadsBack) {
  const auto path = std::filesystem::path(u8"a/é.cpp");
  EXPECT_EQ(xsmp::pathText(path), "a/\xc3\xa9.cpp");
  EXPECT_EQ(xsmp::pathFromText("a/\xc3\xa9.cpp"), path);
}

namespace {

namespace fs = std::filesystem;

// A project folder, with `src-gen` as the generated folder of a generation.
struct GeneratedFolder {
  xsmp::ScratchDirectory scratch;
  fs::path folder = scratch.path / "src-gen";
  std::vector<fs::path> folders{folder};
  xsmp::GeneratedFilesRecord record{folders};

  // A link at `link` to `target`. Relative, as a WASI host refuses an absolute
  // target. False where this platform cannot make one.
  [[nodiscard]] static bool makeLink(const fs::path& target,
                                     const fs::path& link) {
    std::error_code error;
    fs::create_directories(link.parent_path(), error);
    fs::create_symlink(fs::relative(target, link.parent_path()), link, error);
    return !error;
  }
};

}  // namespace

TEST(GeneratedFiles, RemovingStaleFilesKeepsTheRootAndWhatIsOutside) {
  GeneratedFolder project;
  (void)project.scratch.write("src-gen/nested/old.cpp", "generated");
  (void)project.scratch.write("src/hand.cpp", "user code");
  project.record.removeStaleFiles();
  EXPECT_TRUE(fs::is_directory(project.folder));
  EXPECT_TRUE(fs::is_empty(project.folder));
  EXPECT_EQ(xsmp::readFile(project.scratch.path / "src/hand.cpp"), "user code");
}

TEST(GeneratedFiles, StaleFilesAndTheirEmptiedFoldersGo) {
  GeneratedFolder project;
  xsmp::writeFileIfChanged(project.folder / "kept/deep/new.h", "new");
  (void)project.scratch.write("src-gen/kept/deep/stale.h", "stale");
  (void)project.scratch.write("src-gen/old/deep/stale.h", "stale");
  fs::create_directories(project.folder / "kept/empty");
  project.record.removeStaleFiles();
  EXPECT_EQ(xsmp::readFile(project.folder / "kept/deep/new.h"), "new");
  EXPECT_FALSE(fs::exists(project.folder / "kept/deep/stale.h"));
  EXPECT_FALSE(fs::exists(project.folder / "kept/empty"));
  EXPECT_FALSE(fs::exists(project.folder / "old"));
}

TEST(GeneratedFiles, AFileFoundUnchangedStaysAndKeepsItsTime) {
  GeneratedFolder project;
  const auto path = project.scratch.write("src-gen/same.h", "same");
  fs::last_write_time(
      path, fs::file_time_type::clock::now() - std::chrono::hours(24));
  const auto old = fs::last_write_time(path);
  EXPECT_FALSE(xsmp::writeFileIfChanged(path, "same"));
  project.record.removeStaleFiles();
  EXPECT_EQ(fs::last_write_time(path), old);
}

TEST(GeneratedFiles, AFileNotedWithoutAWriteStays) {
  GeneratedFolder project;
  const auto path = project.scratch.write("src-gen/noted.h", "noted");
  xsmp::recordGeneratedFile(path);
  project.record.removeStaleFiles();
  EXPECT_TRUE(fs::exists(path));
}

TEST(GeneratedFiles, WritesFromOtherThreadsAreNoted) {
  GeneratedFolder project;
  xsmp::runInParallel(40, [&](std::size_t index) {
    xsmp::writeFileIfChanged(project.folder / std::to_string(index % 4)
                                 / (std::to_string(index) + ".h"),
                             "generated");
  });
  (void)project.scratch.write("src-gen/stale.h", "stale");
  project.record.removeStaleFiles();
  EXPECT_FALSE(fs::exists(project.folder / "stale.h"));
  for (auto index = 0; index < 40; ++index)
    EXPECT_TRUE(fs::exists(project.folder / std::to_string(index % 4)
                           / (std::to_string(index) + ".h")));
}

TEST(GeneratedFiles, AnotherSpellingOfAPathIsTheSameFile) {
  GeneratedFolder project;
  (void)project.scratch.write("src-gen/n/A.h", "generated");
  (void)project.scratch.write("src-gen/n/stale.h", "stale");
  xsmp::recordGeneratedFile(project.scratch.path / "src-gen" / ".." / "src-gen"
                            / xsmp::pathFromText("n/A.h"));
  EXPECT_TRUE(project.record.noted(project.folder / "n/A.h"));
  EXPECT_EQ(project.record.generatedUnder(project.folder),
            std::vector<fs::path>{xsmp::pathFromText("n/A.h")});
  project.record.removeStaleFiles();
  EXPECT_TRUE(fs::exists(project.folder / "n/A.h"));
  EXPECT_FALSE(fs::exists(project.folder / "n/stale.h"));
}

// Two generators would write it: which one wins could change.
TEST(GeneratedFiles, AGeneratedFileNotedTwiceIsAnError) {
  GeneratedFolder project;
  xsmp::writeFileIfChanged(project.folder / "A.h", "first");
  EXPECT_THROW(xsmp::writeFileIfChanged(project.folder / "A.h", "second"),
               std::runtime_error);
  EXPECT_EQ(xsmp::readFile(project.folder / "A.h"), "first");
  const auto user = project.scratch.path / "src" / "A.cpp";
  xsmp::writeFileIfMissing(user, "stub");
  EXPECT_THROW(xsmp::writeFileIfChanged(user, "generated"), std::runtime_error);
}

// The second ask finds it there.
TEST(GeneratedFiles, AFileOfTheUserIsWrittenOnceWhereItIsMissing) {
  GeneratedFolder project;
  const auto path = project.scratch.path / "src" / "A.cpp";
  EXPECT_TRUE(xsmp::writeFileIfMissing(path, "stub"));
  EXPECT_FALSE(xsmp::writeFileIfMissing(path, "other stub"));
  EXPECT_EQ(xsmp::readFile(path), "stub");
  EXPECT_TRUE(project.record.isUserFile(path));
  (void)project.scratch.write("src/B.cpp", "user code");
  EXPECT_FALSE(
      xsmp::writeFileIfMissing(project.scratch.path / "src" / "B.cpp", "stub"));
  EXPECT_EQ(xsmp::readFile(project.scratch.path / "src" / "B.cpp"),
            "user code");
  EXPECT_TRUE(project.record.generatedUnder(project.scratch.path).empty());
}

// Cleaning would remove it.
TEST(GeneratedFiles, AFileOfTheUserCannotBeInAGeneratedFolder) {
  GeneratedFolder project;
  EXPECT_THROW(xsmp::writeFileIfMissing(project.folder / "A.cpp", "stub"),
               std::runtime_error);
  EXPECT_FALSE(fs::exists(project.folder / "A.cpp"));
}

// A stale source left behind can break a build.
TEST(GeneratedFiles, AStaleFileThatStaysFailsTheCleaning) {
  GeneratedFolder project;
  (void)project.scratch.write("src-gen/locked/stale.cpp", "stale");
  (void)project.scratch.write("src-gen/other.cpp", "stale");
  const auto locked = project.folder / "locked";
  std::error_code error;
  fs::permissions(locked, fs::perms::owner_read | fs::perms::owner_exec,
                  error);
  if (error) GTEST_SKIP() << "No permissions here: " << error.message();
  fs::remove(locked / "stale.cpp", error);
  if (!error) GTEST_SKIP() << "This user can remove from a read-only folder";
  EXPECT_THROW(project.record.removeStaleFiles(), std::runtime_error);
  EXPECT_FALSE(fs::exists(project.folder / "other.cpp"));
  fs::permissions(locked, fs::perms::owner_all);
}

TEST(GeneratedFiles, ASecondNameOfAFileItWroteIsNotStale) {
  // What a filesystem that ignores case shows after a type changes case. A hard
  // link gives one file two names here too.
  GeneratedFolder project;
  const auto written = project.folder / "n" / "NAME.h";
  xsmp::writeFileIfChanged(written, "generated");
  std::error_code error;
  fs::create_hard_link(written, project.folder / "n" / "name.h", error);
  if (error) GTEST_SKIP() << "No hard links here: " << error.message();
  (void)project.scratch.write("src-gen/n/Name.other", "stale");
  project.record.removeStaleFiles();
  EXPECT_TRUE(fs::exists(project.folder / "n" / "name.h"));
  EXPECT_FALSE(fs::exists(project.folder / "n" / "Name.other"));
}

TEST(GeneratedFiles, ADifferentFileWhoseNameDiffersByCaseIsStale) {
  GeneratedFolder project;
  xsmp::writeFileIfChanged(project.folder / "NAME.h", "generated");
  if (fs::exists(project.folder / "name.h"))
    GTEST_SKIP() << "This filesystem ignores case";
  (void)project.scratch.write("src-gen/name.h", "stale");
  project.record.removeStaleFiles();
  EXPECT_FALSE(fs::exists(project.folder / "name.h"));
  EXPECT_TRUE(fs::exists(project.folder / "NAME.h"));
}

TEST(GeneratedFiles, PreparingNeverFollowsALink) {
  GeneratedFolder project;
  const auto external = project.scratch.path / "external";
  (void)project.scratch.write("external/user.cpp", "user code");
  (void)project.scratch.write("src-gen/nested/real.cpp", "generated");
  if (!GeneratedFolder::makeLink(external, project.folder / "nested/link")
      || !GeneratedFolder::makeLink(external, project.folder / "deep/er/link")
      || !GeneratedFolder::makeLink(external / "user.cpp",
                                    project.folder / "file-link.cpp"))
    GTEST_SKIP() << "No links here";
  const fs::path folders[]{project.folder, project.folder};
  xsmp::prepareGeneratedFolders(folders);
  EXPECT_FALSE(fs::symlink_status(project.folder / "nested/link").type()
               == fs::file_type::symlink);
  EXPECT_FALSE(fs::exists(project.folder / "deep/er/link"));
  EXPECT_FALSE(fs::exists(project.folder / "file-link.cpp"));
  EXPECT_EQ(xsmp::readFile(project.folder / "nested/real.cpp"), "generated");
  EXPECT_EQ(xsmp::readFile(external / "user.cpp"), "user code");
}

TEST(GeneratedFiles, ALinkOrAFileAtTheFolderPathGoes) {
  GeneratedFolder project;
  const auto external = project.scratch.path / "external";
  (void)project.scratch.write("external/user.cpp", "user code");
  (void)project.scratch.write("file-gen", "not a folder");
  const fs::path folders[]{project.folder, project.scratch.path / "file-gen",
                           project.scratch.path / "missing-gen"};
  if (GeneratedFolder::makeLink(external, project.folder)) {
    xsmp::prepareGeneratedFolders(folders);
    EXPECT_FALSE(fs::exists(project.folder));
    EXPECT_FALSE(fs::exists(project.scratch.path / "file-gen"));

    // So a write goes to a real folder, and the external one stays as it is.
    xsmp::writeFileIfChanged(project.folder / "user.cpp", "generated");
    EXPECT_TRUE(fs::is_directory(fs::symlink_status(project.folder)));
    EXPECT_EQ(xsmp::readFile(external / "user.cpp"), "user code");
  }
}

TEST(GeneratedFiles, EachEntryIsTriedBeforeAFailureIsReported) {
  GeneratedFolder project;
  const auto external = project.scratch.path / "external";
  if (!GeneratedFolder::makeLink(external, project.folder / "link"))
    GTEST_SKIP() << "No links here";
  (void)project.scratch.write("blocked", "not a folder");
  const fs::path folders[]{project.scratch.path / "blocked/child",
                           project.folder};
  EXPECT_THROW(xsmp::prepareGeneratedFolders(folders), std::runtime_error);
  EXPECT_FALSE(fs::exists(fs::symlink_status(project.folder / "link")));
}

TEST(GeneratedFiles, AFileWhereAFolderGoesGivesWay) {
  GeneratedFolder project;
  (void)project.scratch.write("src-gen/n", "a file where a folder goes");
  EXPECT_TRUE(
      xsmp::writeFileIfChanged(project.folder / "n/m/A.h", "generated"));
  EXPECT_EQ(xsmp::readFile(project.folder / "n/m/A.h"), "generated");
}

TEST(GeneratedFiles, AFolderWhereAFileGoesGivesWay) {
  GeneratedFolder project;
  (void)project.scratch.write("src-gen/A.h/nested/stale", "in the way");
  EXPECT_TRUE(xsmp::writeFileIfChanged(project.folder / "A.h", "generated"));
  EXPECT_EQ(xsmp::readFile(project.folder / "A.h"), "generated");
}

TEST(GeneratedFiles, ManyWritesCanMeetTheSameObstacle) {
  GeneratedFolder project;
  (void)project.scratch.write("src-gen/n", "a file where a folder goes");
  xsmp::runInParallel(16, [&](std::size_t index) {
    xsmp::writeFileIfChanged(project.folder / "n" / std::to_string(index),
                             "generated");
  });
  for (auto index = 0; index < 16; ++index)
    EXPECT_TRUE(xsmp::fileExists(project.folder / "n" / std::to_string(index)));
}

TEST(GeneratedFiles, AnObstacleOutsideTheFoldersStillFails) {
  GeneratedFolder project;
  (void)project.scratch.write("python/n", "a file where a folder goes");
  (void)project.scratch.write("python/A.py/user", "a folder where a file goes");
  EXPECT_THROW(xsmp::writeFileIfChanged(project.scratch.path / "python/n/A.py",
                                        "generated"),
               std::runtime_error);
  EXPECT_THROW(xsmp::writeFileIfChanged(project.scratch.path / "python/A.py",
                                        "generated"),
               std::runtime_error);
  EXPECT_EQ(xsmp::readFile(project.scratch.path / "python/n"),
            "a file where a folder goes");
  EXPECT_EQ(xsmp::readFile(project.scratch.path / "python/A.py/user"),
            "a folder where a file goes");
}

TEST(GeneratedFiles, NothingIsNotedOutsideAGeneration) {
  xsmp::ScratchDirectory scratch;
  const auto folder = scratch.path / "src-gen";
  xsmp::writeFileIfChanged(folder / "before.h", "written");
  {
    const fs::path folders[]{folder};
    xsmp::GeneratedFilesRecord record(folders);
    EXPECT_FALSE(record.noted(folder / "before.h"));
  }
  xsmp::writeFileIfChanged(folder / "after.h", "written");
  (void)scratch.write("src-gen/n", "a file");
  EXPECT_THROW(xsmp::writeFileIfChanged(folder / "n/A.h", "written"),
               std::runtime_error);
}
