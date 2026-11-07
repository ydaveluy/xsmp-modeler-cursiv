#pragma once

#include <concepts>
#include <xsmp/DeploymentGrammarCommon.hpp>

namespace xsmp {

// The configuration language (`.xsmpcfg`): field values to apply to component
// instances.
struct Xsmpcfg : cursiv::extends<DeploymentRules> {
  template <class>
  struct override;

  // A field may be named like a word that starts a statement of a
  // configuration.
  struct StatementWord
      : rule<assign<&xsmp::ConcretePathNamedSegment::reference>(kw<"include">)
             + name_goes_on> {};

  template <class T>
    requires std::same_as<T, DeploymentRules::StatementPath>

  struct override<T> : rule<word_path<StatementWord> | ref<Path>> {};

  // A structure value designates its fields by one name, not by a path.
  struct CfgConcretePathNamedSegment
      : rule<option(flag<&xsmp::ConcretePathNamedSegment::unsafe>(kw<"unsafe">))
             + assign<&xsmp::ConcretePathNamedSegment::reference>(
                 ref<LocalName>)> {};

  struct CfgStructureFieldValue
      : rule<assign<&xsmp::CfgStructureFieldValue::field>(
                 ref<CfgConcretePathNamedSegment>)
             + lit<"=">
             + assign<&xsmp::CfgStructureFieldValue::value>(ref<Value>)> {};

  template <class T>
    requires std::same_as<T, DeploymentRules::StructureValue>

  struct override<T>
      : rule<create<xsmp::StructureValue> + lit<"{">
             + many(append<&xsmp::StructureValue::elements>(
                        ref<CfgStructureFieldValue> | ref<Value>),
                    lit<",">)
             + lit<"}">> {};

  struct ComponentConfiguration;

  struct ConfigurationUsage
      : rule<kw<"include">.doc<R"(`include <configuration> [at <path>]`

Includes another configuration document, optionally below a child component path.
Use `at` to reuse a common configuration on a nested instance instead of at the current level.

**Example**
```xsmp
include PowerNominal at power
```)">() + assign<&xsmp::ConfigurationUsage::configuration>(ref<ValidID>)
             + option(kw<"at">
                      + assign<&xsmp::ConfigurationUsage::path>(ref<Path>))> {};

  struct ComponentConfiguration
      : rule<assign<&xsmp::ComponentConfiguration::name>(ref<StatementPath>)
             + option(lit<":">
                      + assign<&xsmp::ComponentConfiguration::context>(
                          ref<QualifiedName>))
             + lit<"{">
             + many(append<&xsmp::ComponentConfiguration::elements>(
                 ref<ConfigurationUsage> | ref<ComponentConfiguration>
                 | ref<FieldValue>))
             + lit<"}">> {};

  struct Configuration
      : rule<create<xsmp::Configuration>
             + kw<"configuration">.doc<R"(`configuration <name>`

Declares a reusable XSMP configuration document.
A configuration groups field values, nested component configurations and includes that can be applied to matching component instances.

**Example**
```xsmp
configuration Nominal
/platform: demo.Platform
{
    mode = demo.foundation.Mode.Nominal
}
```)">() + assign<&xsmp::NamedElement::name>(ref<ValidID>)
             + many(append<&xsmp::Configuration::elements>(
                 ref<ConfigurationUsage> | ref<ComponentConfiguration>))> {};

  using entry = Configuration;
};

}  // namespace xsmp
