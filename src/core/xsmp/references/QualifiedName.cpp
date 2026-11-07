#include <cursiv/syntax-tree/AstNode.hpp>
#include <xsmp/ast.hpp>
#include <xsmp/references/QualifiedName.hpp>

namespace xsmp {

std::string qualifiedName(const cursiv::AstNode& element) {
  const auto* named = cursiv::ast_cast<const NamedElement>(&element);
  if (named == nullptr) {
    return {};
  }
  std::string name(named->name);
  for (const auto* container = element.parent; container != nullptr;
       container = container->parent) {
    // The catalogue is not part of the qualified name.
    if (cursiv::ast_cast<const Catalogue>(container) != nullptr) {
      break;
    }
    if (const auto* namedContainer =
            cursiv::ast_cast<const NamedElement>(container);
        namedContainer != nullptr) {
      name = std::string(namedContainer->name) + "." + name;
    }
  }
  return name;
}

bool hasQualifiedName(const cursiv::AstNode& element, std::string_view name) {
  const auto* named = cursiv::ast_cast<const NamedElement>(&element);
  if (named == nullptr) return name.empty();
  if (!name.ends_with(named->name)) return false;
  name.remove_suffix(named->name.size());
  for (const auto* container = element.parent; container != nullptr;
       container = container->parent) {
    if (cursiv::ast_cast<const Catalogue>(container) != nullptr) break;
    const auto* outer = cursiv::ast_cast<const NamedElement>(container);
    if (outer == nullptr) continue;
    if (!name.ends_with('.')) return false;
    name.remove_suffix(1);
    if (!name.ends_with(outer->name)) return false;
    name.remove_suffix(outer->name.size());
  }
  return name.empty();
}

}  // namespace xsmp
