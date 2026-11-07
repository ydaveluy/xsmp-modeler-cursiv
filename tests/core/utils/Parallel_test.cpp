#include <gtest/gtest.h>

#include <atomic>
#include <stdexcept>
#include <vector>
#include <xsmp/utils/Parallel.hpp>

TEST(Parallel, RunsEachIndexOnce) {
  std::vector<std::atomic<int>> calls(100);
  xsmp::runInParallel(calls.size(), [&](std::size_t index) { ++calls[index]; });
  for (const auto& count : calls) EXPECT_EQ(count, 1);
}

TEST(Parallel, RunsNothingForNoTasks) {
  xsmp::runInParallel(0, [](std::size_t) { FAIL(); });
}

TEST(Parallel, RunsEveryTaskAndRethrowsTheLowestFailure) {
  std::atomic<int> calls{0};
  try {
    xsmp::runInParallel(20, [&](std::size_t index) {
      ++calls;
      if (index % 5 == 3) throw std::runtime_error(std::to_string(index));
    });
    FAIL() << "A failure must come back";
  } catch (const std::runtime_error& error) {
    EXPECT_STREQ(error.what(), "3");
  }
  EXPECT_EQ(calls, 20);
}
