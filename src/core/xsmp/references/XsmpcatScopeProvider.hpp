#pragma once

#include <cursiv/references/DefaultScopeProvider.hpp>
#include <xsmp/XsmpcatCoreServices.hpp>

namespace xsmp {

// Scoping of a catalogue. A name is looked up in the enclosing elements, each
// with what it inherits, then in the documents the project makes visible, where
// `Smp.` and `Attributes.` may be left out of it.
class XsmpcatScopeProvider final : public cursiv::DefaultScopeProvider {
 public:
  explicit XsmpcatScopeProvider(const XsmpcatCoreServices& services)
      : cursiv::DefaultScopeProvider(services), _xsmpServices(services) {}

  const cursiv::AstNodeDescription* getScopeEntry(
      const cursiv::ReferenceInfo&, std::string_view name) const override;

  bool visitScopeEntries(
      const cursiv::ReferenceInfo&,
      cursiv::function_ref<bool(const cursiv::AstNodeDescription&)>)
      const override;

  bool visitScopeEntries(
      const cursiv::ReferenceInfo&, std::string_view name,
      cursiv::function_ref<bool(const cursiv::AstNodeDescription&)>)
      const override;

 protected:
  const cursiv::AstNodeDescription* getGlobalScopeEntry(
      const cursiv::ReferenceInfo&, std::string_view, cursiv::AcceptSet,
      const cursiv::AstReflection&, bool) const override;

  bool visitGlobalScopeEntries(
      const cursiv::ReferenceInfo&, cursiv::AcceptSet,
      const cursiv::AstReflection&,
      cursiv::function_ref<bool(const cursiv::AstNodeDescription&)>)
      const override;

  bool visitGlobalScopeEntries(
      const cursiv::ReferenceInfo&, std::string_view, cursiv::AcceptSet,
      const cursiv::AstReflection&, bool,
      cursiv::function_ref<bool(const cursiv::AstNodeDescription&)>)
      const override;

 private:
  // The entries of the enclosing elements and of what they inherit, innermost
  // first, one per name at each level. An empty name visits them all. The field
  // of a designated initializer looks in the structure it initializes instead.
  bool visitLocal(
      const cursiv::ReferenceInfo&, std::string_view,
      cursiv::function_ref<bool(const cursiv::AstNodeDescription&)>) const;

  // Whether the reference at this site can hold the entry.
  bool acceptEntry(const cursiv::ReferenceInfo&,
                   const cursiv::AstNodeDescription&) const;

  const XsmpcatCoreServices& _xsmpServices;
};

}  // namespace xsmp
