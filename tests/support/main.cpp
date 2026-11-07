#include <gtest/gtest.h>

#include <cstdlib>
#include <cursiv/ExpectFunction.hpp>
#include <string>

// Keep the entry point in the executable: wasi-libc's weak main fallback does
// not pull Google's main from a static archive.
int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  // A failed check of `cursiv::test` is a failure of the test, which goes on.
  cursiv::test::setExpectFunction([](std::string_view actual,
                                     std::string_view expected,
                                     std::string_view message) {
    EXPECT_EQ(std::string(actual), std::string(expected)) << message;
  });
  // Expected output is raw, so it does not depend on a clang-format version.
#if defined(_WIN32)
  _putenv_s("XSMP_CLANG_FORMAT", "none");
#else
  setenv("XSMP_CLANG_FORMAT", "none", 1);
#endif
  return RUN_ALL_TESTS();
}
