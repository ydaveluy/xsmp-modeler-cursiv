#include <gtest/gtest.h>

#include <chrono>
#include <string>
#include <vector>
#include <xsmp/utils/Process.hpp>

// A WASI module runs these through its host, so they cover the framing too.
namespace {

using xsmp::ProcessError;

// A bare `sh`, so that each test also finds its program through PATH.
std::vector<std::string> shell(std::string script) {
  return {"sh", "-c", std::move(script)};
}

class Process : public testing::Test {
 protected:
  void SetUp() override {
#if defined(_WIN32)
    GTEST_SKIP() << "These tests use a POSIX shell";
#endif
  }
};

TEST_F(Process, ReturnsOutputErrorsAndExitCode) {
  const auto result =
      xsmp::runProcess(shell("cat; echo problem >&2; exit 3"), "input");
  EXPECT_EQ(result.exitCode, 3);
  EXPECT_EQ(result.output, "input");
  EXPECT_EQ(result.errors, "problem\n");
}

TEST_F(Process, KeepsEveryByte) {
  const std::string bytes("\0\xff\xfe text\r\n\0", 12);
  EXPECT_EQ(xsmp::runProcess(shell("cat"), bytes).output, bytes);
}

TEST_F(Process, PassesEachArgumentAsItIs) {
  const auto result = xsmp::runProcess(
      std::vector<std::string>{"sh", "-c", "printf '%s|' \"$@\"", "sh", "a b",
                               "", "-assume-filename", "/c d/e.cpp"},
      {});
  EXPECT_EQ(result.output, "a b||-assume-filename|/c d/e.cpp|");
}

TEST_F(Process, HasNoProblemWithAProcessThatIgnoresItsInput) {
  const auto result =
      xsmp::runProcess(shell("exit 4"), std::string(1024 * 1024, 'x'));
  EXPECT_EQ(result.exitCode, 4);
}

TEST_F(Process, ReportsAProgramItCannotFind) {
  const std::vector<std::string> command{"xsmp-no-such-program", "--version"};
  try {
    (void)xsmp::runProcess(command, {});
    FAIL() << "The program does not exist";
  } catch (const ProcessError& error) {
    EXPECT_EQ(error.kind(), ProcessError::Kind::CannotStart);
  }
}

TEST_F(Process, ReportsASignalAsMinusOne) {
  EXPECT_EQ(xsmp::runProcess(shell("kill -9 $$"), {}).exitCode, -1);
}

TEST_F(Process, StopsAtTheTimeout) {
  const auto started = std::chrono::steady_clock::now();
  try {
    (void)xsmp::runProcess(shell("sleep 30"), {},
                           {.timeout = std::chrono::milliseconds(100)});
    FAIL() << "The process must time out";
  } catch (const ProcessError& error) {
    EXPECT_EQ(error.kind(), ProcessError::Kind::TimedOut);
  }
  EXPECT_LT(std::chrono::steady_clock::now() - started,
            std::chrono::seconds(20));
}

TEST_F(Process, StopsAtTheOutputLimit) {
  for (const auto* stream : {"", " >&2"}) {
    try {
      (void)xsmp::runProcess(
          shell(std::string("head -c 100000 /dev/zero") + stream), {},
          {.maxOutputBytes = 1000});
      FAIL() << "The output is over the limit";
    } catch (const ProcessError& error) {
      EXPECT_EQ(error.kind(), ProcessError::Kind::OutputTooLarge);
    }
  }
}

}  // namespace
