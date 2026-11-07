#pragma once

#include <xsmp/ast.hpp>

namespace xsmp {

// A return parameter is a separate grammar node, but @usage Parameter also
// applies to it. It intentionally does not inherit NamedElement's validation.
[[nodiscard]] inline std::string_view attributeUsageName(
    const cursiv::AstNode& node) noexcept {
  return node.type == &cursiv::ast_type_tag<ReturnParameter>
             ? cursiv::ast_type_tag<Parameter>.name
             : node.type->name;
}

// Whether `category` names the AST type of `node` or one of its bases.
[[nodiscard]] inline bool matchesUsageCategory(
    const cursiv::AstNode& node, std::string_view category) noexcept {
  if (node.type == &cursiv::ast_type_tag<ReturnParameter>)
    return category == cursiv::ast_type_tag<Parameter>.name;
  if (node.type->name == category) return true;
  for (std::uint32_t i = 0; i < node.type->baseCount; ++i)
    if (node.type->bases[i]->name == category) return true;
  return false;
}

}  // namespace xsmp
