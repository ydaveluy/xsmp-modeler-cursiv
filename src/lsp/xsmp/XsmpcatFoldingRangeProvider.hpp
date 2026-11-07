#pragma once

#include <cursiv/ranges/DefaultFoldingRangeProvider.hpp>
#include <xsmp/ast.hpp>

namespace xsmp {

// Catalogue folding: namespaces, types and block comments; the members of a
// type do not fold.
class XsmpcatFoldingRangeProvider final
    : public cursiv::DefaultFoldingRangeProvider {
 public:
  using DefaultFoldingRangeProvider::DefaultFoldingRangeProvider;

 protected:
  bool shouldProcess(const cursiv::AstNode& node) const override {
    return cursiv::ast_cast<Namespace>(&node) || cursiv::ast_cast<Type>(&node);
  }

  bool shouldProcessContent(const cursiv::AstNode& node) const override {
    return cursiv::ast_cast<Catalogue>(&node)
           || cursiv::ast_cast<Namespace>(&node);
  }
};

}  // namespace xsmp
