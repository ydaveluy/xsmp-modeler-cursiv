#pragma once

#include <optional>
#include <string>
#include <string_view>

namespace cursiv {

struct AstNode;
class CommentProvider;

}  // namespace cursiv

namespace xsmp {

/// The first comment of `catalogueNode`'s document, when it is not a doc
/// comment (`/** */`): a license or copyright banner rather than the
/// documentation of the first declaration. Expands `${year}`, `${user}`,
/// `${date}` (`2026-01-31`) and `${time}` (`13:45:09`) before adding
/// `linePrefix` in front of every line.
[[nodiscard]] std::optional<std::string> getCopyrightNotice(
    const cursiv::CommentProvider& commentProvider,
    const cursiv::AstNode& catalogueNode, std::string_view linePrefix = {});

}  // namespace xsmp
