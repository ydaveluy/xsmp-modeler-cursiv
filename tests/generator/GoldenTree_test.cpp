#include "support/GoldenTree.hpp"

#include <xsmp/utils/GenerationStamp.hpp>

namespace {

using xsmp::describeDifference;
using xsmp::FileTree;

}  // namespace

TEST(GoldenTree, TheSameTreesHaveNoDifference) {
  const FileTree tree{{"a.h", "one\n"}, {"b/c.h", "two\n"}};
  EXPECT_EQ(describeDifference(tree, tree), "");
}

TEST(GoldenTree, MissingAndExtraFilesAreBothDifferences) {
  EXPECT_EQ(describeDifference({{"gone.h", ""}}, {{"new.h", ""}}),
            "Missing from the output: gone.h\nNot in the golden: new.h\n");
}

TEST(GoldenTree, TheFirstDifferingFileAndLineAreShown) {
  EXPECT_EQ(
      describeDifference({{"a.h", "same\ngolden\n"}, {"b.h", "golden\n"}},
                         {{"a.h", "same\nactual\n"}, {"b.h", "actual\n"}}),
      "a.h, line 2\n  golden: golden\n  actual: actual\n"
      "and 1 more differing files.\n");
}

TEST(GoldenTree, ALineMissingAtTheEndAndUnreadableBytesAreShown) {
  EXPECT_EQ(describeDifference({{"a.h", "one\n"}}, {{"a.h", "one\n\xE9\t\n"}}),
            "a.h, line 2\n  golden: <end of file>\n  actual: \\xE9\\x09\n");
}

TEST(GoldenTree, OnlyTheLastLineEndingCanDiffer) {
  EXPECT_EQ(describeDifference({{"a.h", "one\n"}}, {{"a.h", "one"}}),
            "a.h, line 2\n  golden: <end of file>\n  actual: <end of file>\n"
            "  (the last line ending differs)\n");
}

TEST(GoldenTree, ThePinnedStampIsReadAndThenRestored) {
  const auto* before = std::getenv("SOURCE_DATE_EPOCH");
  {
    const xsmp::PinnedGenerationStamp pinned;
    EXPECT_EQ(xsmp::reproducibleTime(), std::time_t{1769867109});
    EXPECT_EQ(xsmp::generationUser(), "golden-user");
  }
  EXPECT_EQ(std::getenv("SOURCE_DATE_EPOCH"), before);
}

// What is not a number of seconds pins nothing.
TEST(GoldenTree, AnInvalidPinIsIgnored) {
  for (const auto* invalid : {"", "soon", "12x"}) {
    const xsmp::PinnedGenerationStamp pinned(invalid);
    EXPECT_EQ(xsmp::reproducibleTime(), std::nullopt) << invalid;
  }
}
