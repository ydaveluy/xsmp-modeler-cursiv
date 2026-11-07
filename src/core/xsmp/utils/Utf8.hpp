#pragma once

#include <algorithm>
#include <cstddef>
#include <cursiv/utils/Utf8.hpp>
#include <format>
#include <string>
#include <string_view>
#include <utility>

namespace xsmp {

// How many bytes the UTF-8 character `text` starts with takes, or 0 when it
// does not start with one. A surrogate, an overlong form and what lies beyond
// U+10FFFF are not UTF-8.
[[nodiscard]] inline std::size_t utf8CharacterSize(std::string_view text) {
  if (text.empty()) return 0;
  const auto lead = static_cast<unsigned char>(text[0]);
  if (lead < 0x80) return 1;
  const auto width = cursiv::utf8_width(text[0]);
  if (lead < 0xC2 || lead > 0xF4 || width > text.size()) return 0;
  const auto second = static_cast<unsigned char>(text[1]);
  if (second < (lead == 0xE0   ? 0xA0
                : lead == 0xF0 ? 0x90
                               : 0x80)
      || second > (lead == 0xED   ? 0x9F
                   : lead == 0xF4 ? 0x8F
                                  : 0xBF))
    return 0;
  for (std::size_t k = 2; k < width; ++k)
    if ((static_cast<unsigned char>(text[k]) & 0xC0) != 0x80) return 0;
  return width;
}

// Where `text` stops being UTF-8, or its size.
[[nodiscard]] inline std::size_t utf8Length(std::string_view text) {
  for (std::size_t i = 0; i < text.size();) {
    const auto size = utf8CharacterSize(text.substr(i));
    if (size == 0) return i;
    i += size;
  }
  return text.size();
}

// Whether `text` only holds ASCII characters.
[[nodiscard]] inline bool isAscii(std::string_view text) {
  unsigned char all = 0;
  for (const unsigned char c : text) all |= c;
  return all < 0x80;
}

// `text` with each byte that is not UTF-8 written `\xHH`, as SMP XML writes it.
// A comment may hold such bytes, and a generated file that copies it may not.
[[nodiscard]] inline std::string utf8Text(std::string_view text) {
  // Nearly all text is ASCII, which needs no look at its characters.
  if (isAscii(text)) return std::string(text);
  std::string result;
  result.reserve(text.size());
  while (!text.empty()) {
    const auto size = utf8CharacterSize(text);
    if (size == 0)
      result +=
          std::format("\\x{:02X}", static_cast<unsigned char>(text.front()));
    else
      result += text.substr(0, size);
    text.remove_prefix(std::max<std::size_t>(size, 1));
  }
  return result;
}

// The same for a text that the caller gives up, which ASCII keeps as it is.
[[nodiscard]] inline std::string utf8Text(std::string&& text) {
  return isAscii(text) ? std::move(text) : utf8Text(std::string_view(text));
}

}  // namespace xsmp
