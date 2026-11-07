#pragma once

#include <array>
#include <cursiv/text/LspPosition.hpp>
#include <cursiv/workspace/Document.hpp>
#include <iomanip>
#include <optional>
#include <random>
#include <sstream>
#include <string>

namespace xsmp {

// A random version 4 UUID, written as `8-4-4-4-12` hexadecimal digits.
inline std::string newUuid() {
  std::random_device random;
  std::array<unsigned char, 16> bytes{};
  for (auto& byte : bytes) byte = static_cast<unsigned char>(random());

  // The bits that say version 4 and the standard variant.
  bytes[6] = static_cast<unsigned char>((bytes[6] & 15) | 64);
  bytes[8] = static_cast<unsigned char>((bytes[8] & 63) | 128);

  std::ostringstream out;
  out << std::hex << std::setfill('0');
  for (std::size_t i = 0; i < bytes.size(); ++i) {
    if (i == 4 || i == 6 || i == 8 || i == 10) out << '-';
    out << std::setw(2) << static_cast<unsigned>(bytes[i]);
  }
  return out.str();
}

// The doc comment right before `start`, if there is one.
inline std::optional<cursiv::CstLeaf> docCommentBefore(
    const cursiv::Document& document, cursiv::TextOffset start) {
  const auto& cst = document.parseResult.cst();
  for (auto rank = cst.leaves_before(start); rank > 0;) {
    const auto leaf = cst.leaf(--rank);
    if (leaf.begin == leaf.end) continue;
    if (const auto text = cst.text(leaf.id);
        leaf.hidden && text.starts_with("/**") && text.ends_with("*/")
        && text.size() > 4)
      return leaf;
    break;
  }
  return std::nullopt;
}

// The edit that gives the declaration at `start` a new `@uuid`. The tag takes
// its own line at the end of the doc comment before it, or else a new comment
// is written.
inline ::lsp::TextEdit uuidEdit(const cursiv::Document& document,
                                cursiv::TextOffset start) {
  const auto& lines = document.textDocument();
  const auto comment = docCommentBefore(document, start);
  const auto at = comment ? comment->begin : start;

  // What stands before on the line, as blanks.
  const auto line =
      lines.offsetAt({.line = lines.positionAt(at).line, .character = 0});
  std::string indentation(lines.getText().substr(line, at - line));
  for (auto& c : indentation)
    if (c != '\t') c = ' ';

  if (!comment) {
    const auto position = cursiv::to_lsp(lines.positionAt(start));
    return {.range = {.start = position, .end = position},
            .newText = "/** @uuid " + newUuid() + " */\n" + indentation};
  }

  std::string text(document.parseResult.cst().text(comment->id));
  const auto close = text.rfind("*/");
  // The blanks before `*/`, kept apart from what the comment says.
  const auto last = text.find_last_not_of(" \t", close - 1);
  text.replace(last + 1, close - last - 1,
               (text[last] == '\n' ? "" : "\n") + indentation + " * @uuid "
                   + newUuid() + "\n" + indentation + " ");
  return {.range = cursiv::to_lsp(lines.rangeAt(comment->begin, comment->end)),
          .newText = std::move(text)};
}

}  // namespace xsmp
