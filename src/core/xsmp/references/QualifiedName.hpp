#pragma once

#include <string>
#include <string_view>

namespace cursiv {

struct AstNode;
}

namespace xsmp {

/// The `.`-joined names of `element`'s enclosing namespaces and types and its
/// own, without the catalogue. Empty when `element` is not a named element.
[[nodiscard]] std::string qualifiedName(const cursiv::AstNode& element);

/// Whether `qualifiedName(element)` is `name`, without building it.
[[nodiscard]] bool hasQualifiedName(const cursiv::AstNode& element,
                                    std::string_view name);

}  // namespace xsmp
