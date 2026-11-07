#include <gtest/gtest.h>

#include <algorithm>
#include <string>
#include <vector>
#include <xsmp/AlphabeticalOrder.hpp>

TEST(AlphabeticalOrder, SortsHeadersAsAReaderWould) {
  std::vector<std::string> headers{"unordered_set",
                                   "Smp/IComposite.h",
                                   "functional",
                                   "TasMdk/Container.h",
                                   "demo/support/CounterArray.h",
                                   "Smp/Publication/ITypeRegistry.h",
                                   "demo/support/Counter.h",
                                   "Smp/PrimitiveTypes.h"};
  std::ranges::sort(headers, xsmp::alphabeticalLess);
  EXPECT_EQ(headers,
            (std::vector<std::string>{
                "demo/support/Counter.h", "demo/support/CounterArray.h",
                "functional", "Smp/IComposite.h", "Smp/PrimitiveTypes.h",
                "Smp/Publication/ITypeRegistry.h", "TasMdk/Container.h",
                "unordered_set"}));
}

TEST(AlphabeticalOrder, OfPunctuationAndCase) {
  std::vector<std::string> names{"ab",  "A", "a_b", "aB", "ab.h",
                                 "a/b", "a", "Ab",  "AB", "a.h"};
  std::ranges::sort(names, xsmp::alphabeticalLess);
  EXPECT_EQ(names, (std::vector<std::string>{"a", "A", "a_b", "a.h", "a/b",
                                             "ab", "aB", "Ab", "AB", "ab.h"}));
}
