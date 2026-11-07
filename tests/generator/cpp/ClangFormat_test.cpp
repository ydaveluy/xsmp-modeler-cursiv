#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <functional>
#include <optional>
#include <stdexcept>
#include <string>
#include <xsmp/cpp/ClangFormat.hpp>
#include <xsmp/utils/FileWriter.hpp>
#include <xsmp/utils/Process.hpp>
#include <xsmp/xsmp-sdk/XsmpSdkGenerator.hpp>

#include "support/Files.hpp"
#include "support/GeneratorTestSupport.hpp"

namespace {

using xsmp::GeneratedCppBatch;
using xsmp::readFile;
using xsmp::writeGeneratedCpp;

void setTool(const std::string& value) {
#if defined(_WIN32)
  _putenv_s("XSMP_CLANG_FORMAT", value.c_str());
#else
  setenv("XSMP_CLANG_FORMAT", value.c_str(), 1);
#endif
}

// What `action` writes on stderr, where a test binary can tell.
std::optional<std::string> stderrOf(const std::function<void()>& action) {
#if GTEST_HAS_STREAM_REDIRECTION
  testing::internal::CaptureStderr();
  action();
  return testing::internal::GetCapturedStderr();
#else
  action();
  return std::nullopt;
#endif
}

// Runs the fake tool from a scratch folder: see its script. Each test has its
// own tool name, as the check of a tool is done once per name.
class ClangFormat : public testing::Test {
 protected:
  void SetUp() override {
#if defined(_WIN32)
    GTEST_SKIP() << "The fake tool is a POSIX shell script";
#endif
    std::filesystem::create_directories(_toolFolder);
    // Relative, as a WASI host refuses a link to an absolute path.
    std::filesystem::create_symlink(
        std::filesystem::relative(std::filesystem::path(XSMP_FIXTURES_DIR)
                                      / "clang-format" / "fake-clang-format",
                                  _toolFolder),
        _toolFolder / "fake-clang-format");
    setTool(xsmp::pathText(_toolFolder / "fake-clang-format"));
  }

  void TearDown() override { setTool("none"); }

  // How many times the tool ran, its check included.
  [[nodiscard]] std::size_t runs() const {
    const auto log = xsmp::readFileIfPresent(_toolFolder / "runs");
    return log ? std::ranges::count(*log, '\n') : 0;
  }

  [[nodiscard]] std::string lastRun() const {
    const auto log = readFile(_toolFolder / "runs");
    return log.substr(log.rfind('\n', log.size() - 2) + 1);
  }

  void makeToolFail() const { (void)scratch.write("tool/fails", ""); }

  void generate(const std::filesystem::path& path, const std::string& content) {
    GeneratedCppBatch batch;
    writeGeneratedCpp(path, content);
    batch.run();
  }

  xsmp::ScratchDirectory scratch;

 private:
  std::filesystem::path _toolFolder = scratch.path / "tool";
};

TEST_F(ClangFormat, WritesWhatTheToolWritesAndNamesTheFile) {
  const auto path = scratch.path / "out" / "A.cpp";
  writeGeneratedCpp(path, "int a;\n");
  EXPECT_EQ(readFile(path), "// formatted\nint a;\n");
  EXPECT_EQ(lastRun(), "-assume-filename " + xsmp::pathText(path) + "\n");
  EXPECT_EQ(xsmp::clangFormatted(path, "int b;\n"), "// formatted\nint b;\n");
}

TEST_F(ClangFormat, BatchWritesWhatTheToolWritesAndNamesTheFile) {
  const auto path = scratch.path / "out" / "A.cpp";
  generate(path, "int a;\n");
  EXPECT_EQ(readFile(path), "// formatted\nint a;\n");
  EXPECT_EQ(lastRun(), "-assume-filename " + xsmp::pathText(path) + "\n");
}

TEST_F(ClangFormat, ChecksEachToolOnce) {
  writeGeneratedCpp(scratch.path / "A.cpp", "int a;\n");
  writeGeneratedCpp(scratch.path / "B.cpp", "int b;\n");
  EXPECT_EQ(runs(), 3u);
}

TEST_F(ClangFormat, MissingToolLeavesTheContentRawWithoutAWord) {
  for (const auto* tool :
       {"xsmp-no-such-clang-format", "/no/such/clang-format"}) {
    setTool(tool);
    const auto output = stderrOf([&] {
      generate(scratch.path / "A.cpp", "int a;\n");
      writeGeneratedCpp(scratch.path / "B.cpp", "int b;\n");
    });
    EXPECT_EQ(readFile(scratch.path / "A.cpp"), "int a;\n");
    EXPECT_EQ(readFile(scratch.path / "B.cpp"), "int b;\n");
    if (output) EXPECT_EQ(*output, "");
  }
}

TEST_F(ClangFormat, FailingToolLeavesTheContentRawWithOneWarning) {
  makeToolFail();
  const auto output = stderrOf([&] {
    GeneratedCppBatch batch;
    writeGeneratedCpp(scratch.path / "A.cpp", "int a;\n");
    writeGeneratedCpp(scratch.path / "B.cpp", "int b;\n");
    batch.run();
  });
  EXPECT_EQ(readFile(scratch.path / "A.cpp"), "int a;\n");
  EXPECT_EQ(readFile(scratch.path / "B.cpp"), "int b;\n");
  if (output) {
    EXPECT_EQ(std::ranges::count(*output, '\n'), 2) << *output;
    EXPECT_NE(output->find("2 generated file(s)"), std::string::npos);
    EXPECT_NE(output->find("broken style"), std::string::npos);
  }
}

TEST_F(ClangFormat, FailureIsNotRemembered) {
  makeToolFail();
  const auto path = scratch.path / "A.cpp";
  (void)stderrOf([&] { generate(path, "int a;\n"); });
  std::filesystem::remove(scratch.path / "tool" / "fails");
  generate(path, "int a;\n");
  EXPECT_EQ(readFile(path), "// formatted\nint a;\n");
}

TEST_F(ClangFormat, ToolWithoutOutputLeavesTheContentRaw) {
  const auto empty = scratch.path / "tool" / "empty";
  std::filesystem::create_symlink(
      std::filesystem::relative("/bin/true", scratch.path / "tool"), empty);
  setTool(xsmp::pathText(empty));
  const auto path = scratch.path / "A.cpp";
  (void)stderrOf([&] { generate(path, "int a;\n"); });
  EXPECT_EQ(readFile(path), "int a;\n");
}

TEST_F(ClangFormat, NoneTurnsFormattingOff) {
  setTool("none");
  generate(scratch.path / "A.cpp", "int a;\n");
  writeGeneratedCpp(scratch.path / "B.cpp", "int b;\n");
  EXPECT_EQ(readFile(scratch.path / "A.cpp"), "int a;\n");
  EXPECT_EQ(readFile(scratch.path / "B.cpp"), "int b;\n");
  EXPECT_EQ(xsmp::clangFormatted(scratch.path / "C.cpp", "int c;\n"),
            "int c;\n");
  EXPECT_EQ(runs(), 0u);
}

TEST_F(ClangFormat, SecondGenerationStartsNoProcess) {
  const auto path = scratch.path / "A.cpp";
  generate(path, "int a;\n");
  const auto before = runs();
  generate(path, "int a;\n");
  EXPECT_EQ(runs(), before);
  EXPECT_EQ(readFile(path), "// formatted\nint a;\n");
}

TEST_F(ClangFormat, FileEditedByHandComesBack) {
  const auto path = scratch.path / "A.cpp";
  generate(path, "int a;\n");
  const auto before = runs();
  (void)scratch.write("A.cpp", "edited by hand\n");
  generate(path, "int a;\n");
  EXPECT_EQ(runs(), before + 1);
  EXPECT_EQ(readFile(path), "// formatted\nint a;\n");
}

TEST_F(ClangFormat, RemovedFileComesBack) {
  const auto path = scratch.path / "A.cpp";
  generate(path, "int a;\n");
  std::filesystem::remove(path);
  generate(path, "int a;\n");
  EXPECT_EQ(readFile(path), "// formatted\nint a;\n");
}

TEST_F(ClangFormat, ChangedContentIsFormattedAgain) {
  const auto path = scratch.path / "A.cpp";
  generate(path, "int a;\n");
  generate(path, "int b;\n");
  EXPECT_EQ(readFile(path), "// formatted\nint b;\n");
}

TEST_F(ClangFormat, EditedStyleFileFormatsAgainAndSameResultKeepsTheFile) {
  const auto path = scratch.path / "out" / "A.cpp";
  generate(path, "int a;\n");
  const auto before = runs();

  std::filesystem::last_write_time(
      path,
      std::filesystem::file_time_type::clock::now() - std::chrono::hours(24));
  const auto old = std::filesystem::last_write_time(path);

  // The tool runs again, for a style file above the file, and writes the same.
  (void)scratch.write(".clang-format", "IndentWidth: 7\n");
  generate(path, "int a;\n");
  EXPECT_EQ(runs(), before + 1);
  EXPECT_EQ(std::filesystem::last_write_time(path), old);
  (void)scratch.write("_clang-format", "IndentWidth: 3\n");
  generate(path, "int a;\n");
  EXPECT_EQ(runs(), before + 2);
  (void)scratch.write("out/.clang-format", "IndentWidth: 5\n");
  generate(path, "int a;\n");
  EXPECT_EQ(runs(), before + 3);
  EXPECT_EQ(std::filesystem::last_write_time(path), old);
}

TEST_F(ClangFormat, GenerationRecordsAFileTheMemoSkips) {
  const auto path = scratch.path / "src-gen" / "A.cpp";
  generate(path, "int a;\n");
  const auto before = runs();
  const std::filesystem::path folders[]{scratch.path / "src-gen"};
  xsmp::GeneratedFilesRecord record(folders);
  generate(path, "int a;\n");
  // The version only, which a generation asks once.
  EXPECT_EQ(runs(), before + 1);
  EXPECT_EQ(lastRun(), "--version\n");
  record.removeStaleFiles();
  EXPECT_TRUE(std::filesystem::exists(path));
}

// A tool updated between two generations may write otherwise.
TEST_F(ClangFormat, EachGenerationAsksTheVersionAndANewOneFormatsAgain) {
  const auto path = scratch.path / "A.cpp";
  const std::filesystem::path folders[]{scratch.path / "src-gen"};
  {
    xsmp::GeneratedFilesRecord record(folders);
    generate(path, "int a;\n");
    generate(scratch.path / "B.cpp", "int b;\n");
  }
  const auto before = runs();
  {
    xsmp::GeneratedFilesRecord record(folders);
    generate(path, "int a;\n");
  }
  EXPECT_EQ(runs(), before + 1);
  (void)scratch.write("tool/version", "fake clang-format 2\n");
  {
    xsmp::GeneratedFilesRecord record(folders);
    generate(path, "int a;\n");
  }
  EXPECT_EQ(runs(), before + 3);
}

// A file of the user is formatted only when it is written: once.
TEST_F(ClangFormat, AFileOfTheUserIsFormattedOnlyWhereItIsMissing) {
  const auto missing = scratch.path / "src" / "A.cpp";
  const auto present = scratch.write("src/B.cpp", "user code\n");
  const auto before = runs();
  {
    GeneratedCppBatch batch;
    xsmp::writeUserCpp(missing, "int a;\n");
    xsmp::writeUserCpp(present, "int b;\n");
    xsmp::writeUserCpp(present, "int b;\n");
    batch.run();
  }
  EXPECT_EQ(readFile(missing), "// formatted\nint a;\n");
  EXPECT_EQ(readFile(present), "user code\n");
  EXPECT_EQ(runs(), before + 2);
}

TEST_F(ClangFormat, BatchWritesOnlyWhenItRuns) {
  const auto path = scratch.path / "A.cpp";
  {
    GeneratedCppBatch batch;
    writeGeneratedCpp(path, "int a;\n");
    EXPECT_FALSE(std::filesystem::exists(path));
  }
  EXPECT_FALSE(std::filesystem::exists(path));

  // The batch has closed: this writes at once.
  writeGeneratedCpp(path, "int a;\n");
  EXPECT_TRUE(std::filesystem::exists(path));
}

TEST_F(ClangFormat, BatchHoldingALotOfTextWritesBeforeItsEnd) {
  const auto first = scratch.path / "A.cpp";
  const auto second = scratch.path / "B.cpp";
  const std::string big(5u * 1024u * 1024u, ' ');
  GeneratedCppBatch batch;
  writeGeneratedCpp(first, big);
  EXPECT_FALSE(std::filesystem::exists(first));
  writeGeneratedCpp(second, big);
  EXPECT_TRUE(std::filesystem::exists(first));
  EXPECT_TRUE(std::filesystem::exists(second));
}

TEST_F(ClangFormat, AGeneratedFileWrittenTwiceIsAnError) {
  const auto path = scratch.path / "A.cpp";
  GeneratedCppBatch batch;
  writeGeneratedCpp(path, "int a;\n");
  EXPECT_THROW(writeGeneratedCpp(path, "int b;\n"), std::runtime_error);
  EXPECT_THROW(xsmp::writeUserCpp(path, "int c;\n"), std::runtime_error);
}

TEST_F(ClangFormat, BatchFormatsManyFiles) {
  GeneratedCppBatch batch;
  for (auto index = 0; index < 20; ++index)
    writeGeneratedCpp(scratch.path / (std::to_string(index) + ".cpp"),
                      "int a" + std::to_string(index) + ";\n");
  batch.run();
  for (auto index = 0; index < 20; ++index)
    EXPECT_EQ(readFile(scratch.path / (std::to_string(index) + ".cpp")),
              "// formatted\nint a" + std::to_string(index) + ";\n");
}

TEST_F(ClangFormat, GeneratorWritesThroughABatch) {
  xsmp::ToolFixture fixture;
  const auto& catalogue = fixture.parse(R"(catalogue Demo
namespace demo {
  public struct Point {
    field Smp.Int32 x
  }
}
)");
  xsmp::XsmpSdkGenerator generator(*fixture.language);
  generator.generate(catalogue, fixture.directory);
  EXPECT_TRUE(fixture.read("src-gen/Demo.h").starts_with("// formatted\n"));
  EXPECT_TRUE(
      fixture.read("src-gen/demo/Point.h").starts_with("// formatted\n"));

  const auto before = runs();
  generator.generate(catalogue, fixture.directory);
  EXPECT_EQ(runs(), before);
}

TEST(RealClangFormat, ObeysTheStyleFileOfTheProject) {
  setTool("clang-format");
  const std::array<std::string, 2> version{"clang-format", "--version"};
  auto present = false;
  try {
    present = xsmp::runProcess(version, {}).exitCode == 0;
  } catch (const xsmp::ProcessError&) {
  }
  if (!present) {
    setTool("none");
    GTEST_SKIP() << "clang-format is not installed";
  }

  xsmp::ScratchDirectory project;
  (void)project.write(".clang-format", "BasedOnStyle: LLVM\nIndentWidth: 7\n");
  const auto path = project.path / "src-gen" / "A.cpp";
  {
    GeneratedCppBatch batch;
    writeGeneratedCpp(path, "void f( ) { if (a) {\nb( );\n} }\n");
    batch.run();
  }
  EXPECT_EQ(readFile(path),
            "void f() {\n       if (a) {\n              b();\n       }\n}\n");
  setTool("none");
}

}  // namespace
