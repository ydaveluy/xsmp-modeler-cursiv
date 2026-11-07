#pragma once

#include <string_view>

namespace xsmp {

/// Whether `left` comes before `right` in alphabetical order, as a reader would
/// sort them: case only counts between otherwise equal texts, lower case first,
/// and punctuation comes before digits, which come before letters.
[[nodiscard]] bool alphabeticalLess(std::string_view left,
                                    std::string_view right);

}  // namespace xsmp
