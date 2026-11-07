#pragma once

#include <cursiv/grammar/GrammarSpec.hpp>

namespace xsmp {

// The trivia of every XSMP language.
struct CommonRules : cursiv::GrammarSpec {
  struct WS : terminal<some(s)> {};

  struct SL_COMMENT : terminal<lit<"//"> + many(cr<R"(^\r\n)">)> {};

  struct ML_COMMENT : terminal<through(lit<"/*">, lit<"*/">)> {};

  using ignored = WS;
  using hidden = list<ML_COMMENT, SL_COMMENT>;

  // Inside a string or a character, a backslash takes the next character with
  // it, so that `\"` does not close the text. `decoded_text` decides what the
  // pair means. Neither holds a line break: `\n` writes one.
  static constexpr auto escape = lit<"\\"> + cr<R"(^\n\r)">;
  static constexpr auto string_text = many(escape | cr<R"(^"\\\n\r)">);
  static constexpr auto char_text = many(escape | cr<R"(^'\\\n\r)">);
};

}  // namespace xsmp
