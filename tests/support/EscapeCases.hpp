#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

// Every form a Char8 or a String8 literal can take, under the rules of C++:
// what it holds, and how each output spells it. The compiler reads the C++
// spelling here too, so a spelling that does not hold the value fails where
// this table stands.

namespace xsmp::escapes {

struct Case {
  std::string_view source;    // as written in a catalogue
  std::string_view cpp;       // as generated in C++
  std::string value;          // the bytes it holds
  std::string_view imported;  // as written back from SMP XML

  [[nodiscard]] bool isChar() const { return source.front() == '\''; }

  // A deployment document has no concatenation.
  [[nodiscard]] bool isConcatenated() const {
    return source.find("\" \"") != std::string_view::npos;
  }
};

inline std::string bytesOf(char c) { return std::string(1, c); }

template <std::size_t N>
std::string bytesOf(const char (&text)[N]) {
  return std::string(text, N - 1);
}

#define XSMP_ESCAPE(source, cpp, imported) \
  Case { source, #cpp, bytesOf(cpp), imported }

// Written back as it was written.
#define XSMP_ESCAPE_SAME(source, cpp) XSMP_ESCAPE(source, cpp, source)

inline const std::vector<Case>& cases() {
  static const std::vector<Case> table{
      // Short escapes.
      XSMP_ESCAPE_SAME(R"("\n")", "\n"),
      XSMP_ESCAPE_SAME(R"("\t")", "\t"),
      XSMP_ESCAPE_SAME(R"("\r")", "\r"),
      XSMP_ESCAPE_SAME(R"("\b")", "\b"),
      XSMP_ESCAPE_SAME(R"("\f")", "\f"),
      XSMP_ESCAPE_SAME(R"("\v")", "\v"),
      XSMP_ESCAPE_SAME(R"("\a")", "\a"),
      XSMP_ESCAPE_SAME(R"("\\")", "\\"),
      XSMP_ESCAPE_SAME(R"("\0")", "\0"),
      XSMP_ESCAPE_SAME(R"("\"")", "\""),
      XSMP_ESCAPE(R"("\'")", "'", R"("'")"),
      XSMP_ESCAPE(R"("\?")", "?", R"("?")"),
      XSMP_ESCAPE_SAME(R"('\n')", '\n'),
      XSMP_ESCAPE_SAME(R"('\t')", '\t'),
      XSMP_ESCAPE_SAME(R"('\r')", '\r'),
      XSMP_ESCAPE_SAME(R"('\b')", '\b'),
      XSMP_ESCAPE_SAME(R"('\f')", '\f'),
      XSMP_ESCAPE_SAME(R"('\v')", '\v'),
      XSMP_ESCAPE_SAME(R"('\a')", '\a'),
      XSMP_ESCAPE_SAME(R"('\\')", '\\'),
      XSMP_ESCAPE_SAME(R"('\0')", '\0'),
      XSMP_ESCAPE_SAME(R"('\'')", '\''),
      XSMP_ESCAPE(R"('\"')", '"', R"('"')"),
      XSMP_ESCAPE(R"('\?')", '?', R"('?')"),

      // Octal takes at most three digits, and names a byte.
      XSMP_ESCAPE(R"("\1")", "\001", R"("\001")"),
      XSMP_ESCAPE(R"('\1')", '\001', R"('\001')"),
      XSMP_ESCAPE(R"("\12")", "\n", R"("\n")"),
      XSMP_ESCAPE(R"("\101")", "A", R"("A")"),
      XSMP_ESCAPE(R"('\101')", 'A', R"('A')"),
      XSMP_ESCAPE_SAME(R"("\177")", "\177"),
      XSMP_ESCAPE_SAME(R"("\377")", "\377"),
      XSMP_ESCAPE_SAME(R"('\377')", '\377'),
      XSMP_ESCAPE_SAME(R"('\200')", '\200'),
      XSMP_ESCAPE(R"("\1018")", "A8", R"("A8")"),
      XSMP_ESCAPE(R"("\1234")", "S4", R"("S4")"),
      XSMP_ESCAPE(R"("\018")", "\0018", R"("\0018")"),
      XSMP_ESCAPE(R"("\000")", "\0", R"("\0")"),
      XSMP_ESCAPE(R"("\08")", "\0008", R"("\0008")"),
      XSMP_ESCAPE(R"("\09")", "\0009", R"("\0009")"),

      // Hexadecimal takes every digit that follows, and names a byte.
      XSMP_ESCAPE(R"("\x41")", "A", R"("A")"),
      XSMP_ESCAPE(R"('\x41')", 'A', R"('A')"),
      XSMP_ESCAPE(R"("\x00")", "\0", R"("\0")"),
      XSMP_ESCAPE(R"('\x00')", '\0', R"('\0')"),
      XSMP_ESCAPE(R"("\x01")", "\001", R"("\001")"),
      XSMP_ESCAPE(R"('\x01')", '\001', R"('\001')"),
      XSMP_ESCAPE(R"("\x4")", "\004", R"("\004")"),
      XSMP_ESCAPE(R"('\x4')", '\004', R"('\004')"),
      XSMP_ESCAPE(R"("\x4G")", "\004G", R"("\004G")"),
      XSMP_ESCAPE(R"("\x1f")", "\037", R"("\037")"),
      XSMP_ESCAPE(R"("\x7f")", "\177", R"("\177")"),
      XSMP_ESCAPE(R"('\x7F')", '\177', R"('\177')"),
      XSMP_ESCAPE(R"("\xff")", "\377", R"("\377")"),
      XSMP_ESCAPE(R"('\xff')", '\377', R"('\377')"),
      XSMP_ESCAPE(R"('\xFF')", '\377', R"('\377')"),
      XSMP_ESCAPE(R"('\x80')", '\200', R"('\200')"),
      XSMP_ESCAPE(R"("\xC3\xA9")", "é", R"("é")"),
      XSMP_ESCAPE(R"("\x0041")", "A", R"("A")"),
      XSMP_ESCAPE(R"("\x00F")", "\017", R"("\017")"),
      XSMP_ESCAPE(R"("\x005")", "\005", R"("\005")"),
      XSMP_ESCAPE(R"("\x00a")", "\n", R"("\n")"),
      XSMP_ESCAPE(R"("\x001")", "\001", R"("\001")"),

      // Bytes that are not UTF-8, in the middle, at the end and before digits.
      XSMP_ESCAPE(R"("a\xFFz")", "a\377z", R"("a\377z")"),
      XSMP_ESCAPE(R"("a\377b")", "a\377b", R"("a\377b")"),
      XSMP_ESCAPE(R"("a\xFF")", "a\377", R"("a\377")"),
      XSMP_ESCAPE(R"("\xFF" "12")", "\37712", R"("\37712")"),
      XSMP_ESCAPE(R"("\xC3")", "\303", R"("\303")"),
      XSMP_ESCAPE(R"("\xC3" "a")", "\303a", R"("\303a")"),
      XSMP_ESCAPE(R"("\xE2\x82")", "\342\202", R"("\342\202")"),
      XSMP_ESCAPE(R"("\xED\xA0\x80")", "\355\240\200", R"("\355\240\200")"),
      XSMP_ESCAPE(R"("\xC0\x80")", "\300\200", R"("\300\200")"),
      XSMP_ESCAPE(R"("\xF4\x90\x80\x80")", "\364\220\200\200",
                  R"("\364\220\200\200")"),
      XSMP_ESCAPE(R"("é\xFF")", "é\377", R"("é\377")"),
      XSMP_ESCAPE(R"("\xFFé")", "\377é", R"("\377é")"),
      XSMP_ESCAPE_SAME(R"("\377\377")", "\377\377"),

      // Unicode gives the UTF-8 bytes of a character.
      XSMP_ESCAPE(R"("\u0041")", "A", R"("A")"),
      XSMP_ESCAPE(R"('\u0041')", 'A', R"('A')"),
      XSMP_ESCAPE(R"("\u00e9")", "é", R"("é")"),
      XSMP_ESCAPE(R"("\u20AC")", "€", R"("€")"),
      XSMP_ESCAPE(R"("\U0001F600")", "😀", R"("😀")"),
      XSMP_ESCAPE(R"("\u0000")", "\0", R"("\0")"),
      XSMP_ESCAPE(R"("\U00000000")", "\0", R"("\0")"),
      XSMP_ESCAPE(R"("\u00001")", "\0001", R"("\0001")"),
      XSMP_ESCAPE(R"("\u0041F")", "AF", R"("AF")"),
      XSMP_ESCAPE_SAME(R"("\uFFFE")", "\uFFFE"),
      XSMP_ESCAPE_SAME(R"("\uFFFF")", "\uFFFF"),

      // An unknown escape keeps its character alone: see `unknownEscapes`.
      XSMP_ESCAPE(R"("\q")", "q", R"("q")"),
      XSMP_ESCAPE(R"('\q')", 'q', R"('q')"),
      XSMP_ESCAPE(R"("\d")", "d", R"("d")"),
      XSMP_ESCAPE(R"("C:\dir")", "C:dir", R"("C:dir")"),
      XSMP_ESCAPE(R"("\8")", "8", R"("8")"),
      XSMP_ESCAPE(R"("\ ")", " ", R"(" ")"),
      XSMP_ESCAPE(R"("\é")", "é", R"("é")"),

      // Raw text.
      XSMP_ESCAPE_SAME(R"("é€😀")", "é€😀"),
      XSMP_ESCAPE("\"\t\"", "\t", R"("\t")"),
      XSMP_ESCAPE("'\t'", '\t', R"('\t')"),
      XSMP_ESCAPE("\"\x01\"", "\001", R"("\001")"),
      XSMP_ESCAPE("'\x7F'", '\177', R"('\177')"),
      XSMP_ESCAPE("\"\xEF\xBF\xBE\"", "\uFFFE", R"("\uFFFE")"),
      XSMP_ESCAPE("\"\xEF\xBF\xBF\"", "\uFFFF", R"("\uFFFF")"),

      // No text, and more than one character.
      XSMP_ESCAPE_SAME(R"("")", ""),
      XSMP_ESCAPE_SAME(R"("ab")", "ab"),
      XSMP_ESCAPE_SAME(R"("\0\0")", "\0\0"),
      XSMP_ESCAPE_SAME(R"("\n\n")", "\n\n"),
      XSMP_ESCAPE_SAME(R"("\a\b\f\v")", "\a\b\f\v"),

      // Text that reads as an escape once decoded.
      XSMP_ESCAPE_SAME(R"("\\x00")", "\\x00"),
      XSMP_ESCAPE_SAME(R"("\\0")", "\\0"),
      XSMP_ESCAPE_SAME(R"("\\n")", "\\n"),
      XSMP_ESCAPE(R"("\x5C")", "\\", R"("\\")"),
      XSMP_ESCAPE(R"('\x5C')", '\\', R"('\\')"),
      XSMP_ESCAPE(R"("\x5Cx00")", "\\x00", R"("\\x00")"),
      XSMP_ESCAPE(R"("\134x00")", "\\x00", R"("\\x00")"),
      XSMP_ESCAPE_SAME(R"("\\x5C")", "\\x5C"),
      XSMP_ESCAPE_SAME(R"("\\\\x00")", "\\\\x00"),
      XSMP_ESCAPE_SAME(R"("\\x1f")", "\\x1f"),
      XSMP_ESCAPE_SAME(R"("\\x41")", "\\x41"),
      XSMP_ESCAPE_SAME(R"("\\xZZ")", "\\xZZ"),
      XSMP_ESCAPE_SAME(R"("\\x")", "\\x"),
      XSMP_ESCAPE_SAME(R"("\\u0041")", "\\u0041"),
      XSMP_ESCAPE_SAME(R"("\\xEF\\xBF\\xBE")", "\\xEF\\xBF\\xBE"),
      XSMP_ESCAPE_SAME(R"("\\xef\\xbf\\xbf")", "\\xef\\xbf\\xbf"),
      XSMP_ESCAPE_SAME(R"("\\xEF\\xBF\uFFFE")", "\\xEF\\xBF\uFFFE"),
      XSMP_ESCAPE(R"("\\\x01")", "\\\001", R"("\\\001")"),
      XSMP_ESCAPE(R"("\\x\x01")", "\\x\001", R"("\\x\001")"),
      XSMP_ESCAPE_SAME(R"("C:\\dir\\new")", "C:\\dir\\new"),
      XSMP_ESCAPE_SAME(R"("\\xFF")", "\\xFF"),
      XSMP_ESCAPE_SAME(R"("\\377")", "\\377"),
      XSMP_ESCAPE_SAME(R"("\\xC3\\xA9")", "\\xC3\\xA9"),

      // NUL, in the middle and at the end, and before what could extend it.
      XSMP_ESCAPE_SAME(R"("a\0b")", "a\0b"),
      XSMP_ESCAPE_SAME(R"("a\0")", "a\0"),
      XSMP_ESCAPE_SAME(R"("a\0\0b")", "a\0\0b"),
      XSMP_ESCAPE_SAME(R"("\0a")", "\0a"),
      XSMP_ESCAPE_SAME(R"("\0f")", "\0f"),
      XSMP_ESCAPE_SAME(R"("\0\n")", "\0\n"),
      XSMP_ESCAPE_SAME(R"("\0\\")", "\0\\"),
      XSMP_ESCAPE(R"("\0\061")", "\0001", R"("\0001")"),
      XSMP_ESCAPE_SAME(R"("\0000")", "\0000"),
      XSMP_ESCAPE_SAME(R"("\0008")", "\0008"),
      XSMP_ESCAPE_SAME(R"("\0009")", "\0009"),

      // What XML gives a meaning, or normalises.
      XSMP_ESCAPE_SAME(R"("<")", "<"),
      XSMP_ESCAPE_SAME(R"('<')", '<'),
      XSMP_ESCAPE_SAME(R"("&")", "&"),
      XSMP_ESCAPE_SAME(R"('&')", '&'),
      XSMP_ESCAPE_SAME(R"(">")", ">"),
      XSMP_ESCAPE_SAME(R"("'")", "'"),
      XSMP_ESCAPE_SAME(R"('"')", '"'),
      XSMP_ESCAPE_SAME(R"("say \"hi\"")", "say \"hi\""),
      XSMP_ESCAPE_SAME(R"("]]>")", "]]>"),
      XSMP_ESCAPE_SAME(R"("&amp;&#10;")", "&amp;&#10;"),
      XSMP_ESCAPE_SAME(R"("<!-- <![CDATA[")", "<!-- <![CDATA["),
      XSMP_ESCAPE_SAME(R"(" a  b ")", " a  b "),
      XSMP_ESCAPE_SAME(R"(" ")", " "),
      XSMP_ESCAPE_SAME(R"("  ")", "  "),
      XSMP_ESCAPE_SAME(R"("\r\n\t")", "\r\n\t"),
      XSMP_ESCAPE_SAME(R"(" \n ")", " \n "),

      // A trigraph holds its bytes in any C++ standard.
      XSMP_ESCAPE(R"("??=")", "?\?=", R"("??=")"),
      XSMP_ESCAPE(R"("??/")", "?\?/", R"("??/")"),
      XSMP_ESCAPE(R"("??'")", "?\?'", R"("??'")"),
      XSMP_ESCAPE(R"("??-")", "?\?-", R"("??-")"),
      XSMP_ESCAPE(R"("???=")", "??\?=", R"("???=")"),
      XSMP_ESCAPE(R"("?\?=")", "?\?=", R"("??=")"),
      XSMP_ESCAPE_SAME(R"("??")", "??"),
      XSMP_ESCAPE_SAME(R"("? ?=")", "? ?="),

      // Each part of a concatenation is decoded on its own.
      XSMP_ESCAPE(R"("a" "b")", "ab", R"("ab")"),
      XSMP_ESCAPE(R"("" "")", "", R"("")"),
      XSMP_ESCAPE(R"("\0" "1")", "\0001", R"("\0001")"),
      XSMP_ESCAPE(R"("\0" "9")", "\0009", R"("\0009")"),
      XSMP_ESCAPE(R"("\x00" "F")", "\0F", R"("\0F")"),
      XSMP_ESCAPE(R"("\x41" "B")", "AB", R"("AB")"),
      XSMP_ESCAPE(R"("\x4" "1")", "\0041", R"("\0041")"),
      XSMP_ESCAPE(R"("\\" "x00")", "\\x00", R"("\\x00")"),
      XSMP_ESCAPE(R"("\1" "01")", "\00101", R"("\00101")"),
      XSMP_ESCAPE(R"("?" "?=")", "?\?=", R"("??=")"),
      XSMP_ESCAPE(R"("C:\\" "\"")", "C:\\\"", R"("C:\\\"")"),
  };
  return table;
}

#undef XSMP_ESCAPE_SAME
#undef XSMP_ESCAPE

// What is refused, and what is said of it. Each literal stands alone, since one
// that is not closed takes what follows.
struct Refused {
  std::string_view source;
  std::string_view message;
};

inline constexpr std::string_view notOneChar =
    "A 'Char8' shall contain exactly one character.";
inline constexpr std::string_view noHexDigit =
    R"('\x' needs a hexadecimal digit)";
inline constexpr std::string_view fourDigits =
    R"('\u' needs 4 hexadecimal digits)";
inline constexpr std::string_view eightDigits =
    R"('\U' needs 8 hexadecimal digits)";
inline constexpr std::string_view rawBytes =
    R"(This literal is not valid UTF-8: use an escape such as \xE9.)";

inline constexpr Refused refused[] = {
    {R"("\x")", noHexDigit},
    {R"('\x')", noHexDigit},
    {R"("\xZZ")", noHexDigit},
    {R"("\u41")", fourDigits},
    {R"("\u004")", fourDigits},
    {R"("\uZZZZ")", fourDigits},
    {R"("\U0041")", eightDigits},
    {R"("\U0000041")", eightDigits},
    {R"("\x414")", R"(Escape '\x414' is out of range)"},
    {R"("a\xFFb")", R"(Escape '\xFFb' is out of range)"},
    {R"("\x41F")", R"(Escape '\x41F' is out of range)"},
    {R"("\x1F1")", R"(Escape '\x1F1' is out of range)"},
    {R"('\x100')", R"(Escape '\x100' is out of range)"},
    {R"("\xFFFFFFFFFFFFFFFFFF")",
     R"(Escape '\xFFFFFFFFFFFFFFFFFF' is out of range)"},
    {R"("\400")", R"(Escape '\400' is out of range)"},
    {R"('\400')", R"(Escape '\400' is out of range)"},
    {R"("\777")", R"(Escape '\777' is out of range)"},
    {R"("\uD800")", R"(Escape '\uD800' is not a Unicode character)"},
    {R"("\uDFFF")", R"(Escape '\uDFFF' is not a Unicode character)"},
    {R"("\uD83D\uDE00")", R"(Escape '\uD83D' is not a Unicode character)"},
    {R"("\U00110000")", R"(Escape '\U00110000' is not a Unicode character)"},
    {R"("\UFFFFFFFF")", R"(Escape '\UFFFFFFFF' is not a Unicode character)"},
    {R"("\x" "1")", noHexDigit},
    {R"("abc\")", R"(Expecting STRING but found '"abc\"'.)"},
    {R"("C:\")", R"(Expecting STRING but found '"C:\"'.)"},
    {R"('\')", R"(Expecting CHAR but found ''\''.)"},
    {R"("abc)", "Expecting STRING but found '\"abc'."},
    {"\"a\nb\"", "Expecting STRING but found '\"a b\"'."},
    {"'a\nb'", "Expecting CHAR but found ''a b''."},
    {"\"\\\n\"", "Expecting STRING but found '\"\\ \"'."},
    {"''", notOneChar},
    {"'ab'", notOneChar},
    {R"('\0\0')", notOneChar},
    {R"('\n\n')", notOneChar},
    {"'é'", notOneChar},
    {"'€'", notOneChar},
    {"'😀'", notOneChar},
    {R"('\xC3\xA9')", notOneChar},
    {R"('\u00e9')", notOneChar},
    {R"('\uFFFE')", notOneChar},
    {R"('\U0001F600')", notOneChar},
    {R"('\\x00')", notOneChar},
    {"\"caf\xE9\"", rawBytes},
    {"\"\xE9\"", rawBytes},
    {"'\xE9'", rawBytes},
    {"\"a\\\xE9\"", rawBytes},
    {"\"\xFF\xFE\"", rawBytes},
    {"\"\xC3\"", rawBytes},
};

// Each escape that C++ does not know has its warning, and a literal holds as
// many as it has of them.
inline constexpr Refused unknownEscapes[] = {
    {R"("\q")", R"(Unknown escape '\q')"},
    {R"('\q')", R"(Unknown escape '\q')"},
    {R"("\d")", R"(Unknown escape '\d')"},
    {R"("C:\dir")", R"(Unknown escape '\d')"},
    {R"("\8")", R"(Unknown escape '\8')"},
    {R"("\ ")", R"(Unknown escape '\ ')"},
    {R"("\é")", R"(Unknown escape '\é')"},
};

}  // namespace xsmp::escapes
