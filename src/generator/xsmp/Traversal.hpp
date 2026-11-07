#pragma once

#include <algorithm>
#include <cursiv/syntax-tree/DocumentReferences.hpp>
#include <cursiv/workspace/Document.hpp>
#include <set>
#include <vector>
#include <xsmp/ast.hpp>
#include <xsmp/utils/TreeUtils.hpp>
#include <xsmp/workspace/SourceUri.hpp>

namespace xsmp {

// The nodes below `node` that are a `T`, parents first.
template <class T>
std::vector<const T*> allContent(const cursiv::AstNode& node) {
  std::vector<const T*> result;
  for (auto* child : descendantsOf(node))
    if (auto* value = cursiv::ast_cast<const T>(child)) result.push_back(value);

  return result;
}

// The Dependencies of the package of `catalogue`: each other catalogue that it
// refers to, sorted by name. Not a builtin library, and not a catalogue only
// reached through an Interface, which has no implementation.
inline std::vector<const Catalogue*> packageDependencies(
    const Catalogue& catalogue) {
  const auto& parsed = documentOf(catalogue).parseResult;
  std::set<const Catalogue*> dependencies;
  if (parsed.references)
    parsed.references->visitResolved(
        parsed.cst(), [&](cursiv::TextOffset, cursiv::TextOffset,
                          const cursiv::AstNode& target, bool) {
          if (cursiv::ast_cast<const Interface>(&target)) return;
          const auto* document = target.getDocument();
          if (!document || isBuiltinUri(document->uri)) return;
          const auto* other =
              cursiv::ast_cast<const Catalogue>(document->parseResult.value);
          if (other && other != &catalogue) dependencies.insert(other);
        });

  std::vector<const Catalogue*> result(dependencies.begin(),
                                       dependencies.end());
  std::ranges::sort(result, [](const auto* a, const auto* b) {
    return a->name != b->name ? a->name < b->name
                              : documentOf(*a).uri < documentOf(*b).uri;
  });
  return result;
}

}  // namespace xsmp
