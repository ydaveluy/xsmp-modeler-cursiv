#pragma once

#include <xsmp/DeploymentGrammarCommon.hpp>

namespace xsmp {

// The link base language (`.xsmplnk`): connections between component instances,
// written apart from the assembly.
struct Xsmplnk : cursiv::extends<DeploymentRules> {
  // At the start of a block, a link kind can also be the first name of its path.
  struct StatementWord
      : rule<assign<&xsmp::ConcretePathNamedSegment::reference>(
                 kw<"event"> | kw<"field"> | kw<"interface">)
             + &(path_dot | lit<"/"> | lit<"["> | lit<"{">)> {};

  struct ComponentLinkBase
      : rule<assign<&xsmp::ComponentLinkBase::name>(word_path<StatementWord> | ref<Path>)
             + lit<"{">
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
