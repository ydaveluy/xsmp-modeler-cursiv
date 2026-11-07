#pragma once

#include <concepts>
#include <xsmp/DeploymentGrammarCommon.hpp>

namespace xsmp {

// The link base language (`.xsmplnk`): connections between component instances,
// written apart from the assembly.
struct Xsmplnk : cursiv::extends<DeploymentRules> {
  template <class>
  struct override;

  // A path may be the root alone, so what follows it can start the next
  // statement: these words are read as a name only when a name goes on.
  template <class T>
    requires std::same_as<T, DeploymentRules::ValidID>

  struct override<T>
      : rule<ref<ID> | kw<"event"> | kw<"field"> | kw<"interface">> {};

  struct ComponentLinkBase
      : rule<assign<&xsmp::ComponentLinkBase::name>(ref<Path>) + lit<"{">
             + many(append<&xsmp::ComponentLinkBase::elements>(
                 ref<Link> | ref<ComponentLinkBase>))
             + lit<"}">> {};

  struct LinkBase
      : rule<create<xsmp::LinkBase>
             + kw<"link">.doc<R"(`link <name> [for <assembly>]`

Declares a standalone link base document.
A link base groups reusable connection definitions between component instances and can optionally anchor itself on an assembly to type-check templated paths.

**Example**
```xsmp
link OrbitalLinks for OrbitalSegment
/avionics
{
}
```)">() + assign<&xsmp::NamedElement::name>(ref<ValidID>)
             + option(kw<"for">
                      + assign<&xsmp::LinkBase::assembly>(ref<ValidID>))
             + many(
                 append<&xsmp::LinkBase::elements>(ref<ComponentLinkBase>))> {};

  using entry = LinkBase;
};

}  // namespace xsmp
