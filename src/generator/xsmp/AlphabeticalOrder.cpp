#include <algorithm>
#include <array>
#include <cctype>
#include <xsmp/AlphabeticalOrder.hpp>

namespace xsmp {

namespace {

// Where each byte comes in the alphabetical order. A byte out of ASCII comes
// after all of them.
constexpr auto alphabeticalRanks = [] {
  constexpr std::string_view order =
      "\t\n\v\f\r _-,;:!?.'\"()[]{}@*/\\&#%`^+<=>|~$"
      "0123456789abcdefghijklmnopqrstuvwxyz";
  std::array<int, 256> ranks{};
  for (std::size_t byte = 0; byte < ranks.size(); ++byte) {
    const auto lower = byte >= 'A' && byte <= 'Z' ? byte - 'A' + 'a' : byte;
    const auto at = order.find(static_cast<char>(lower));
    ranks[byte] = static_cast<int>(
        at == std::string_view::npos ? order.size() + byte : at);
  }
  return ranks;
}();

int alphabeticalRank(char c) {
  return alphabeticalRanks[static_cast<unsigned char>(c)];
}

}  // namespace

bool alphabeticalLess(std::string_view left, std::string_view right) {
  const auto size = std::min(left.size(), right.size());
  for (std::size_t i = 0; i < size; ++i)
    if (const auto a = alphabeticalRank(left[i]),
        b = alphabeticalRank(right[i]);
        a != b)
      return a < b;
  if (left.size() != right.size()) return left.size() < right.size();
  for (std::size_t i = 0; i < size; ++i)
    if (left[i] != right[i])
      return std::islower(static_cast<unsigned char>(left[i])) != 0;
  return false;
}

}  // namespace xsmp
