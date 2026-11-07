#pragma once

#include <cursiv/hierarchy/TypeHierarchyProvider.hpp>
#include <cursiv/services/CoreServices.hpp>

namespace xsmp {

// The editor's type hierarchy view for catalogues, which Cursiv leaves to the
// language. Supertypes are what a type declares; subtypes are the language
// types that name it.
class XsmpcatTypeHierarchyProvider final
    : public cursiv::TypeHierarchyProvider {
 public:
  explicit XsmpcatTypeHierarchyProvider(const cursiv::CoreServices& services)
      : _core(services) {}

  std::vector<::lsp::TypeHierarchyItem> prepareTypeHierarchy(
      const cursiv::Document&, ::lsp::Position,
      const cursiv::CancellationToken& = {}) const override;

  std::vector<::lsp::TypeHierarchyItem> supertypes(
      const ::lsp::TypeHierarchyItem&,
      const cursiv::CancellationToken& = {}) const override;

  std::vector<::lsp::TypeHierarchyItem> subtypes(
      const ::lsp::TypeHierarchyItem&,
      const cursiv::CancellationToken& = {}) const override;

 private:
  const cursiv::CoreServices& _core;
};

}  // namespace xsmp
