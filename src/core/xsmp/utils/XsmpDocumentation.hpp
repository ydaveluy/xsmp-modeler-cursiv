#pragma once

#include <cstdint>
#include <cursiv/documentation/DocComment.hpp>
#include <cursiv/text/TextRange.hpp>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace cursiv {

struct AstNode;
struct CoreServices;
class CommentProvider;

}  // namespace cursiv

namespace xsmp {

/// A doc-tag's trimmed content plus its ABSOLUTE document byte range (of the
/// tag's own content, e.g. the UUID string itself) — for diagnostics that
/// need to anchor precisely on the tag rather than the whole declaration.
/// `std::uint32_t` matches `cursiv::TextOffset` exactly (kept as a plain
/// integer here so this header doesn't need to include cursiv's CST headers).
struct DocTagRange {
  std::string content;
  std::uint32_t begin = 0;
  std::uint32_t end = 0;
};

// What a node's leading doc comment (`/** ... */`) says. Positions are byte
// offsets in the document, which a diagnostic's `.range(begin, end)` takes.

/// The doc comment of `node` as a whole, or `std::nullopt` when it has none.
[[nodiscard]] std::optional<cursiv::DocComment> docComment(
    const cursiv::AstNode& node);

/// The trimmed content of the first block `@tagName` tag, or `std::nullopt`
/// when there is no doc comment or no such tag. An inline `{@tagName ...}`
/// never matches.
[[nodiscard]] std::optional<std::string> docTag(const cursiv::AstNode& node,
                                                std::string_view tagName);

/// Same lookup as `docTag`, with the range of the tag's content.
[[nodiscard]] std::optional<DocTagRange> docTagRange(
    const cursiv::AstNode& node, std::string_view tagName);

/// Every block `@tagName` tag in document order, for tags that may repeat such
/// as `@usage`.
[[nodiscard]] std::vector<DocTagRange> docTagRanges(const cursiv::AstNode& node,
                                                    std::string_view tagName);

/// The description before block tags, or the operation's @param/@return text
/// for a parameter. Built-in functions and constants use their standard
/// descriptions.
[[nodiscard]] std::optional<std::string> docDescription(
    const cursiv::AstNode& node);

}  // namespace xsmp
