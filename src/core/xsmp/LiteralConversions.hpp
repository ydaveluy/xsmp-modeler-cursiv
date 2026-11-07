#pragma once

#include <algorithm>
#include <cstdint>
#include <format>
#include <string>
#include <string_view>
#include <utility>

#include "cursiv/grammar/Conversion.hpp"
#include "xsmp/utils/Utf8.hpp"

namespace xsmp {

// Whether C++ gives a meaning to a backslash before `c`.
inline constexpr bool isKnownEscape(char c) {
  return std::string_view(R"('"?\abfnrtvxuU01234567)").find(c)
         != std::string_view::npos;
}

// A STRING or CHAR is the text between its quotes, as it is written. A raw byte
// that is not UTF-8 is refused, since what it stands for is unknown.
inline constexpr auto quoted_text =
    [](std::string_view text) -> cursiv::ConversionResult<std::string_view> {
  using Result = cursiv::ConversionResult<std::string_view>;
  text = text.substr(1, text.size() - 2);
  if (utf8Length(text) < text.size())
    return Result::err(
        R"(This literal is not valid UTF-8: use an escape such as \xE9.)");
  return Result::ok(text);
};

// The value of that text, as C++ reads it with UTF-8 as its execution charset:
// bytes, not always valid UTF-8. `\n` is one newline, and `\\`, `\"`, `\'` and
// `\?` stand for themselves. `\x` with all the hexadecimal digits that follow and
// octal with at most three digits name one byte, so `"\xC3\xA9"` is `é` and
// `\x414` is out of range. `\u` with 4 digits and `\U` with 8 give the UTF-8 bytes
// of a character. Any other escape keeps the character alone, as GCC does:
// `checkEscapes` warns about it.
inline constexpr auto decoded_text =
    [](std::string_view text) -> cursiv::ConversionResult<std::string> {
  using Result = cursiv::ConversionResult<std::string>;
  if (text.find('\\') == std::string_view::npos)
    return Result::ok(std::string(text));

  // An escape never decodes to more bytes than it is written in, so the size of
  // the text is enough room.
  std::string value;
  value.reserve(text.size());

  const auto hex = [](char c) -> int {
    return c >= '0' && c <= '9'   ? c - '0'
           : c >= 'a' && c <= 'f' ? c - 'a' + 10
           : c >= 'A' && c <= 'F' ? c - 'A' + 10
                                  : -1;
  };

  const auto outOfRange = [&](std::size_t begin, std::size_t end) {
    return Result::err("Escape '" + std::string(text.substr(begin, end - begin))
                       + "' is out of range");
  };

  // Writes a code point as UTF-8.
  const auto put = [&](std::uint32_t c) {
    if (c < 0x80)
      value += static_cast<char>(c);
    else if (c < 0x800) {
      value += static_cast<char>(0xC0 | (c >> 6));
      value += static_cast<char>(0x80 | (c & 63));
    } else if (c < 0x10000) {
      value += static_cast<char>(0xE0 | (c >> 12));
      value += static_cast<char>(0x80 | ((c >> 6) & 63));
      value += static_cast<char>(0x80 | (c & 63));
    } else {
      value += static_cast<char>(0xF0 | (c >> 18));
      value += static_cast<char>(0x80 | ((c >> 12) & 63));
      value += static_cast<char>(0x80 | ((c >> 6) & 63));
      value += static_cast<char>(0x80 | (c & 63));
    }
  };

  for (std::size_t i = 0; i < text.size();) {
    const auto begin = i;
    auto c = text[i++];
    if (c != '\\') {
      value += c;
      continue;
    }

    // Only a text the parser repaired ends here, and it has said so.
    if (i == text.size()) break;
    c = text[i++];
    switch (c) {
      case 'a':
        value += '\a';
        break;
      case 'b':
        value += '\b';
        break;
      case 'f':
        value += '\f';
        break;
      case 'n':
        value += '\n';
        break;
      case 'r':
        value += '\r';
        break;
      case 't':
        value += '\t';
        break;
      case 'v':
        value += '\v';
        break;
      case 'x': {
        const auto first = i;
        // Past one byte the value stops growing, so any length fits.
        std::uint32_t byte = 0;
        for (; i < text.size() && hex(text[i]) >= 0; ++i)
          byte = std::min<std::uint32_t>(
              byte * 16 + static_cast<std::uint32_t>(hex(text[i])), 0x100);
        if (i == first) return Result::err("'\\x' needs a hexadecimal digit");
        if (byte > 0xFF) return outOfRange(begin, i);
        value += static_cast<char>(byte);
        break;
      }
      case 'u':
      case 'U': {
        const std::size_t width = c == 'u' ? 4 : 8;
        std::uint32_t scalar = 0;
        for (std::size_t k = 0; k < width; ++k, ++i) {
          if (i == text.size() || hex(text[i]) < 0)
            return Result::err(
                std::format("'\\{}' needs {} hexadecimal digits", c, width));
          scalar = scalar * 16 + static_cast<std::uint32_t>(hex(text[i]));
        }
        if (scalar > 0x10FFFF || (scalar >= 0xD800 && scalar <= 0xDFFF))
          return Result::err("Escape '"
                             + std::string(text.substr(begin, i - begin))
                             + "' is not a Unicode character");
        put(scalar);
        break;
      }
      default:
        if (c >= '0' && c <= '7') {
          std::uint32_t byte = static_cast<std::uint32_t>(c - '0');
          for (unsigned k = 1;
               k < 3 && i < text.size() && text[i] >= '0' && text[i] <= '7';
               ++k)
            byte = byte * 8 + static_cast<std::uint32_t>(text[i++] - '0');
          if (byte > 0xFF) return outOfRange(begin, i);
          value += static_cast<char>(byte);
        } else
          // `\\`, `\"`, `\'` and `\?`, or an unknown escape: the character.
          value += c;
    }
  }

  return Result::ok(std::move(value));
};

// A STRING or CHAR that is only ever read decoded.
inline constexpr auto quoted_value =
    [](std::string_view text) -> cursiv::ConversionResult<std::string> {
  const auto written = quoted_text(text);
  if (!written)
    return cursiv::ConversionResult<std::string>::err(written.error.message);
  return decoded_text(*written.value);
};

// An integer literal as C++ spells it. A time-suffixed one is written in
// nanoseconds, with digits grouped by thousands and an `L` suffix, so `5s` reads
// `5'000'000'000L`. A leading zero means octal, with or without a unit, and the
// result keeps that base. The digits are multiplied one by one, so any length
// fits.
inline std::string integer_value(std::string_view text) {
  // The last digits of these forms could read as a unit: `0xd`.
  if (text.starts_with("0x") || text.starts_with("0X") || text.starts_with("0b")
      || text.starts_with("0B"))
    return std::string(text);

  std::uint64_t factor = 0;
  std::size_t suffix = 1;
  if (text.ends_with("ns")) {
    factor = 1;
    suffix = 2;
  } else if (text.ends_with("us")) {
    factor = 1000;
    suffix = 2;
  } else if (text.ends_with("ms")) {
    factor = 1000000;
    suffix = 2;
  } else if (text.ends_with("mn")) {
    factor = 60000000000ULL;
    suffix = 2;
  } else if (text.ends_with('s'))
    factor = 1000000000;
  else if (text.ends_with('h'))
    factor = 3600000000000ULL;
  else if (text.ends_with('d'))
    factor = 86400000000000ULL;

  if (!factor) return std::string(text);

  // Multiplied from the last digit, so `reversed` holds the result backwards.
  const auto digits = text.substr(0, text.size() - suffix);
  const auto octal = digits.size() > 1 && digits.front() == '0';
  const std::uint64_t base = octal ? 8 : 10;
  std::string reversed;
  reversed.reserve(digits.size() + 16);
  std::uint64_t carry = 0;
  for (std::size_t i = digits.size(); i > 0;) {
    const char c = digits[--i];
    if (c == '\'') continue;
    const std::uint64_t product =
        static_cast<unsigned>(c - '0') * factor + carry;
    reversed += static_cast<char>('0' + product % base);
    carry = product / base;
  }
  while (carry) {
    reversed += static_cast<char>('0' + carry % base);
    carry /= base;
  }
  while (reversed.size() > 1 && reversed.back() == '0') reversed.pop_back();

  // The digits in reading order, grouped by thousands.
  std::string result;
  result.reserve(reversed.size() + reversed.size() / 3 + 2);
  if (octal) result += '0';
  for (auto i = reversed.size(); i > 0;) {
    result += reversed[--i];
    if (i && i % 3 == 0) result += '\'';
  }
  result += 'L';
  return result;
}

}  // namespace xsmp
