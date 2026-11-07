#pragma once

#include <concepts>
#include <xsmp/DeploymentGrammarCommon.hpp>

namespace xsmp {

// The assembly language (`.xsmpasb`): a tree of model instances, with their
// field values, links and calls.
struct Xsmpasb : cursiv::extends<DeploymentRules> {
  template <class>
  struct override;

  // A field may be named like a word that starts a statement of a model.
  static constexpr auto statement_words = kw<"event"> | kw<"field">
                                          | kw<"interface"> | kw<"call">
                                          | kw<"property"> | kw<"subscribe">;

  struct StatementWord
      : rule<assign<&xsmp::ConcretePathNamedSegment::reference>(statement_words)
             + name_goes_on> {};

  template <class T>
    requires std::same_as<T, DeploymentRules::StatementPath>

  struct override<T> : rule<word_path<StatementWord> | ref<Path>> {};

  // The dot tells a component name from the bare name of an assembly.
  static constexpr auto dotted_names = some(lit<"."> + ref<ValidID>);

  template <class T>
    requires std::same_as<T, DeploymentRules::QualifiedName>

  struct override<T> : rule<ref<ValidID> + dotted_names> {};

  // An instance name may hold template placeholders.
  struct InstanceName : rule<ref<PATTERN> | ref<ID>> {};

  struct ModelInstance;
  struct AssemblyInstance;
  struct SubInstance;

  // Local names, each accepting one kind of member.
  struct LocalEntryPoint : rule<local_reference<xsmp::EntryPoint>> {};

  struct LocalOperation : rule<local_reference<xsmp::Operation>> {};

  struct LocalProperty : rule<local_reference<xsmp::Property>> {};

  // It starts a statement, as a path does.
  struct LocalContainer
      : rule<local_reference<xsmp::Container>
             | (assign<xref<&xsmp::LocalNamedReference::reference,
                            xsmp::Container>>(statement_words)
                + name_goes_on)> {};

  struct GlobalEventHandler
      : rule<kw<"subscribe">.doc<R"(`subscribe <entrypoint> -> "<global-event>"`

Subscribes one entry point of the configured or current model instance to a simulator-wide global event.

**Example**
```xsmp
subscribe step -> "PlatformReady"
```)">() + assign<&xsmp::GlobalEventHandler::entryPoint>(ref<LocalEntryPoint>)
             + lit<"->">
             + assign<&xsmp::GlobalEventHandler::globalEventName>(
                 ref<DecodedString>)> {};

  struct OperationCall
      : rule<kw<"call">.doc<R"(`call <operation>(<parameter = value, ...>)`

Invokes one published operation of the current configured or current model instance.

**Example**
```xsmp
call apply(requestedMode = demo.foundation.Mode.Nominal)
```)">() + assign<&xsmp::OperationCall::operation>(ref<LocalOperation>)
             + lit<"(">
             + many(
                 append<&xsmp::OperationCall::parameters>(ref<ParameterValue>),
                 lit<",">)
             + lit<")">> {};

  struct PropertyValue
      : rule<kw<"property">.doc<R"(`property <property> = <value>`

Sets one writable property of the current configured or current model instance.

**Example**
```xsmp
property mode = demo.foundation.Mode.Nominal
```)">() + assign<&xsmp::PropertyValue::property>(ref<LocalProperty>)
             + lit<"=">
             + assign<&xsmp::PropertyValue::value>(ref<SimpleValue>)> {};

  struct Invocation : rule<ref<OperationCall> | ref<PropertyValue>> {};

  struct AssemblyComponentConfiguration
      : rule<kw<"configure">.doc<R"(`configure <instance-path> { ... }`

Declares a configuration block for one model or service instance inside the assembly hierarchy.
The path is relative to the containing assembly or sub-assembly instance and may use template placeholders.

**Example**
```xsmp
configure avionics
{
    modeState = demo.foundation.Mode.Nominal
}
```)">() + assign<&xsmp::AssemblyComponentConfiguration::name>(ref<Path>)
             + lit<"{">
             + many(append<&xsmp::AssemblyComponentConfiguration::elements>(
                 ref<GlobalEventHandler> | ref<Invocation> | ref<FieldValue>))
             + lit<"}">> {};

  // The implementation is the name of a component, or a string, which is not
  // linked. Without one, the container of the instance gives it.
  struct ModelInstance
      : rule<create<xsmp::ModelInstance>
             + assign<&xsmp::NamedElement::name>(ref<InstanceName>)
             + option(lit<":">
                      + (assign<&xsmp::ModelInstance::implementation>(
                             ref<QualifiedName>)
                         | assign<&xsmp::ModelInstance::strImplementation>(
                             ref<DecodedString>)))
             + option(lit<"{">
                      + many(append<&xsmp::ModelInstance::elements>(
                          ref<GlobalEventHandler> | ref<SubInstance> | ref<Link>
                          | ref<Invocation> | ref<FieldValue>))
                      + lit<"}">)> {};

  struct AssemblyInstance
      : rule<create<xsmp::AssemblyInstance>
             + assign<&xsmp::NamedElement::name>(ref<InstanceName>) + lit<":">
             + assign<&xsmp::AssemblyInstance::assembly>(ref<ValidID>)
             + option(lit<"<">
                      + many(append<&xsmp::AssemblyInstance::arguments>(
                                 ref<TemplateArgument>),
                             lit<",">)
                      + lit<">">)
             + option(
                 kw<"using"> + kw<"config">
                 + assign<&xsmp::AssemblyInstance::configuration>(ref<ValidID>))
             + option(kw<"using"> + kw<"link">
                      + assign<&xsmp::AssemblyInstance::linkBase>(ref<ValidID>))
             + option(lit<"{">
                      + many(append<&xsmp::AssemblyInstance::elements>(
                          ref<AssemblyComponentConfiguration>))
                      + lit<"}">)> {};

  // `container += instance`.
  struct SubInstance
      : rule<assign<&xsmp::SubInstance::container>(ref<LocalContainer>)
             + lit<"+=">
             + assign<&xsmp::SubInstance::instance>(
                 ref<ModelInstance> | ref<AssemblyInstance>)> {};

  struct Assembly
      : rule<create<xsmp::Assembly>
             + kw<"assembly">.doc<R"(`assembly [<parameter, ...>] <name>`

Declares an XSMP assembly document.
An assembly defines one root model instance, optional template parameters and optional component configurations that override state or behavior inside the assembly tree.

**Example**
```xsmp
assembly <Lane = "Ops"> OrbitalSegment
Root: demo.orbit.Platform
{
}
```)">()
             + option(lit<"<">
                      + some(append<&xsmp::Assembly::parameters>(
                                 ref<TemplateParameter>),
                             lit<",">)
                      + lit<">">)
             + assign<&xsmp::NamedElement::name>(ref<ValidID>)
             + many(append<&xsmp::Assembly::configurations>(
                 ref<AssemblyComponentConfiguration>))
             + assign<&xsmp::Assembly::model>(ref<ModelInstance>)
             + many(append<&xsmp::Assembly::configurations>(
                 ref<AssemblyComponentConfiguration>))> {};

  using entry = Assembly;
};

}  // namespace xsmp
