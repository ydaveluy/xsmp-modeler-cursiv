#pragma once

#include <algorithm>
#include <charconv>
#include <format>
#include <optional>
#include <string>
#include <string_view>
#include <xsmp/utils/Utf8.hpp>

// How a text goes into SMP XML and back. XML 1.0 cannot hold bytes that are not
// UTF-8, most control characters, U+FFFE and U+FFFF, even as a reference, so
// each of their bytes is written `\xHH`. Any other text stays as it is,
// backslashes included, except a backslash that would read as such an escape,
// which is written `\x5C`. An escape names a byte XML cannot hold by itself, a
// byte above 0x7F, or a backslash, so `\x41` is plain text. Decoding therefore
// gives back any text.

namespace xsmp {

namespace detail {

// Whether XML 1.0 cannot hold the control character `byte`.
constexpr bool isControlXmlCannotHold(unsigned char byte) {
  return byte < 0x20 && byte != '\t' && byte != '\n' && byte != '\r';
}

// The byte of the escape that `text` starts with, if it starts with one.
inline std::optional<unsigned char> leadingXmlEscape(std::string_view text) {
  if (text.size() < 4 || !text.starts_with("\\x")) return std::nullopt;
  unsigned value = 0;
  const auto* const end = text.data() + 4;
  const auto [read, error] = std::from_chars(text.data() + 2, end, value, 16);
  if (error != std::errc{} || read != end) return std::nullopt;
  const auto byte = static_cast<unsigned char>(value);
  if (byte != '\\' && byte < 0x80 && !isControlXmlCannotHold(byte))
    return std::nullopt;
  return byte;
}

}  // namespace detail

// `text` as SMP XML holds it: valid UTF-8, and never a NUL.
[[nodiscard]] inline std::string encodeXmlText(std::string_view text) {
  std::string result;
  result.reserve(text.size());
  while (!text.empty()) {
    const auto size = utf8CharacterSize(text);
    const auto character = text.substr(0, std::max<std::size_t>(size, 1));
    if (size == 0
        || detail::isControlXmlCannotHold(
            static_cast<unsigned char>(text.front()))
        || character == "\xEF\xBF\xBE" || character == "\xEF\xBF\xBF") {
      for (const auto byte : character)
        result += std::format("\\x{:02X}", static_cast<unsigned char>(byte));
    } else if (detail::leadingXmlEscape(text)) {
      result += "\\x5C";
    } else {
      result += character;
    }
    text.remove_prefix(character.size());
  }
  return result;
}

// The text that `encodeXmlText` wrote as `written`. Any text that it did not
// write comes back as it is, unless it holds one of its escapes.
[[nodiscard]] inline std::string decodeXmlText(std::string_view written) {
  std::string result;
  result.reserve(written.size());
  while (!written.empty()) {
    if (const auto byte = detail::leadingXmlEscape(written)) {
      result += static_cast<char>(*byte);
      written.remove_prefix(4);
    } else {
      result += written.front();
      written.remove_prefix(1);
    }
  }
  return result;
}

}  // namespace xsmp
