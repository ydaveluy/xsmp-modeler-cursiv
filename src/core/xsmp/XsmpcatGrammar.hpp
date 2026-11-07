#pragma once

#include <xsmp/CommonGrammar.hpp>
#include <xsmp/LiteralConversions.hpp>
#include <xsmp/ast.hpp>

namespace xsmp {

// The catalogue language (`.xsmpcat`): namespaces of SMP types, with the
// constant expressions their values use.
struct Xsmpcat : CommonRules {
  struct ID : terminal<cr<"_a-zA-Z"> + many(w)> {};

  static constexpr auto hex_digit = cr<"0-9a-fA-F">;

  struct STRING
      : terminal<as<quoted_value>(lit<"\""> + string_text + lit<"\"">)> {};

  struct CHAR : terminal<as<quoted_value>(lit<"'"> + char_text + lit<"'">)> {};

  static constexpr auto digit_sequence = d + many(cr<"0-9'">);
  static constexpr auto exponent_part =
      cr<"eE"> + option(cr<"+-">) + digit_sequence;

  // A dot never ends a number before another dot: `1...5` is a range.
  static constexpr auto fractional_constant =
      (option(digit_sequence) + lit<"."> + digit_sequence)
      | (digit_sequence + lit<".", !lit<".">>);

  struct FLOATING_LITERAL
      : terminal<(fractional_constant + option(exponent_part)
                  + option(cr<"fF">))
                 | (digit_sequence + exponent_part + option(cr<"fF">))> {};

  static constexpr auto integer_suffix =
      (cr<"uU"> + option(cr<"lL">)) | (cr<"lL"> + option(cr<"uU">));

  // Hexadecimal and binary forms take a C++ suffix. Decimal and octal forms may
  // take a time unit instead: see `integer_value`.
  struct INTEGER_LITERAL
      : terminal<
            (((lit<"0"> + cr<"xX"> + hex_digit + many(cr<"0-9a-fA-F'">))
              | (lit<"0"> + cr<"bB"> + cr<"01"> + many(cr<"'01">)))
             + option(integer_suffix))
            | (((cr<"1-9"> + many(cr<"0-9'">)) | (lit<"0"> + many(cr<"0-7'">)))
               + option(lit<"ns"> | lit<"us"> | lit<"ms"> | lit<"s"> | lit<"mn">
                        | lit<"h"> | lit<"d"> | integer_suffix))> {};

  // A keyword is only read where the grammar expects it, so a name may spell
  // any word. These start a parameter as well as a type name: `f(in.Foo x)`.
  static constexpr auto valid_id = ref<ID> | kw<"in"> | kw<"out"> | kw<"inout">;
  static constexpr auto more_names = many(lit<"."> + valid_id);

  struct QualifiedName : rule<valid_id + more_names> {};

  struct UnaryOperation;
  struct Literal;

  // The operators and precedence of C++, tightest first.
  struct Expression
      : infix<UnaryOperation, xsmp::BinaryOperation,
              levels(left(lit<"*"> | lit<"/"> | lit<"%">),
                     left(lit<"+"> | lit<"-">), left(lit<"<<"> | lit<">>">),
                     left(lit<">="> | lit<"<="> | lit<">"> | lit<"<">),
                     left(lit<"=="> | lit<"!=">), left(lit<"&">),
                     left(lit<"^">), left(lit<"|">), left(lit<"&&">),
                     left(lit<"||">)),
              &xsmp::BinaryOperation::leftOperand,
              &xsmp::BinaryOperation::feature,
              &xsmp::BinaryOperation::rightOperand> {};

  struct UnaryOperation
      : rule<create<xsmp::UnaryOperation>(
                 assign<&xsmp::UnaryOperation::feature>(lit<"!"> | lit<"-">
                                                        | lit<"+"> | lit<"~">)
                 + assign<&xsmp::UnaryOperation::operand>(ref<UnaryOperation>))
             | ref<Literal>> {};

  struct NamedElementReference
      : rule<assign<&xsmp::NamedElementReference::value>(ref<QualifiedName>)> {
  };

  struct ParenthesizedExpression
      : rule<lit<"(">
             + assign<&xsmp::ParenthesizedExpression::expr>(ref<Expression>)
             + lit<")">> {};

  struct DesignatedInitializer
      : rule<lit<"."> + assign<&xsmp::DesignatedInitializer::field>(valid_id)
             + option(lit<"=">
                      + assign<&xsmp::DesignatedInitializer::expr>(
                          ref<Expression>))> {};

  struct CollectionLiteral
      : rule<lit<"{">
             + many(append<&xsmp::CollectionLiteral::elements>(
                        ref<Expression> | ref<DesignatedInitializer>),
                    lit<",">)
             + lit<"}">> {};

  struct IntegerLiteral
      : rule<assign<&xsmp::IntegerLiteral::text>(ref<INTEGER_LITERAL>)> {};

  struct FloatingLiteral
      : rule<assign<&xsmp::FloatingLiteral::text>(ref<FLOATING_LITERAL>)> {};

  struct StringLiteral
      : rule<some(append<&xsmp::StringLiteral::value>(ref<STRING>))> {};

  struct CharacterLiteral
      : rule<assign<&xsmp::CharacterLiteral::value>(ref<CHAR>)> {};

  struct BuiltInConstant
      : rule<lit<"$"> + assign<&xsmp::BuiltInConstant::name>(ref<ID>)> {};

  struct BuiltInFunction
      : rule<lit<"$"> + assign<&xsmp::BuiltInFunction::name>(ref<ID>) + lit<"(">
             + option(assign<&xsmp::BuiltInFunction::argument>(ref<Expression>))
             + lit<")">> {};

  struct BooleanLiteral
      : rule<create<xsmp::BooleanLiteral>
             + (kw<"false">
                | flag<&xsmp::BooleanLiteral::isTrue>(kw<"true">))> {};

  struct KeywordExpression
      : rule<assign<&xsmp::KeywordExpression::name>(kw<"nullptr">)> {};

  struct Literal
      : rule<ref<CollectionLiteral> | ref<FloatingLiteral> | ref<IntegerLiteral>
             | ref<StringLiteral> | ref<CharacterLiteral> | ref<BooleanLiteral>
             | ref<ParenthesizedExpression> | ref<BuiltInFunction>
             | ref<BuiltInConstant> | ref<KeywordExpression>
             | ref<NamedElementReference>> {};

  struct Attribute;
  struct Constant;
  struct Field;
  struct Property;
  struct Operation;
  struct Association;
  struct Container;
  struct Reference;
  struct EntryPoint;
  struct EventSink;
  struct EventSource;

  static constexpr auto visibility_modifiers =
      kw<"private">.doc<"The element is visible only within its containing classifier.">() |
      kw<"protected">.doc<"The element is visible within its containing "
                        "classifier and derived classifiers thereof.">() |
      kw<"public">.doc<"The element is globally visible.">();

  static constexpr auto field_modifiers =
      visibility_modifiers | kw<"input">.doc<"Input flag of the field.">() |
      kw<"output">.doc<"Output flag of the field.">() |
      kw<"transient">.doc<"Transient flag of the field (opposite of state).">();

  static constexpr auto class_modifiers =
      visibility_modifiers |
      kw<"abstract">.doc<"The type is abstract. Abstract types cannot be "
                        "instantiated, but they can be subtyped.">();

  static constexpr auto property_modifiers =
      kw<"readWrite">.doc<"Specifies a property, which has both a getter and a setter.">() |
      kw<"readOnly">.doc<"Specifies a property, which only has a getter.">() |
      kw<"writeOnly">.doc<"Specifies a property, which only has a setter.">() |
      visibility_modifiers;

  static constexpr auto parameter_direction =
      kw<"in">.doc<"The parameter is read-only to the operation, i.e. its value must "
          "be specified on call, and cannot be changed inside the "
          "operation.">() |
      kw<"out">.doc<"The parameter is write-only to the operation, i.e. its value is "
          "unspecified on call, and must be set by the operation.">() |
      kw<"inout">.doc<"The parameter must be specified on call, and may be "
                     "changed by the operation.">();

  static constexpr auto range_kind =
      lit<"..."> | lit<"..<"> | lit<"<.."> | lit<"<.<">;

  // What every declaration starts with.
  static constexpr auto attributes =
      many(append<&xsmp::NamedElement::attributes>(ref<Attribute>));

  // The modifiers come in any order: validation reports duplicates and
  // conflicts.
  template <auto Modifiers>
  static constexpr auto prefix_with =
      attributes + many(append<&xsmp::VisibilityElement::modifiers>(Modifiers));

  static constexpr auto visibility_prefix = prefix_with<visibility_modifiers>;
  static constexpr auto class_prefix = prefix_with<class_modifiers>;
  static constexpr auto name = assign<&xsmp::NamedElement::name>(valid_id);

  // The `+` and `*` forms, written without brackets.
  struct ShortMultiplicity
      : rule<create<xsmp::Multiplicity>
             + (flag<&xsmp::Multiplicity::aux>(lit<"+">) | lit<"*">)> {};

  // `[n]`, `[lower ... upper]`, `[lower ... *]`, `[+]`, `[*]` or `[]`.
  struct Multiplicity
      : rule<lit<"[">
             + option((assign<&xsmp::Multiplicity::lower>(ref<Expression>)
                       + option(lit<"...">
                                + (flag<&xsmp::Multiplicity::aux>(lit<"*">)
                                   | assign<&xsmp::Multiplicity::upper>(
                                       ref<Expression>))))
                      | flag<&xsmp::Multiplicity::aux>(lit<"+">) | lit<"*">)
             + lit<"]">> {};

  // After the type of a member: `?`, or a multiplicity. Nothing means exactly
  // one.
  static constexpr auto multiplicity_declaration =
      option(flag<&xsmp::NamedElementWithMultiplicity::optional_>(lit<"?">)
             | assign<&xsmp::NamedElementWithMultiplicity::multiplicity>(
                 ref<Multiplicity> | ref<ShortMultiplicity>));

  // The types of a catalogue.

  struct Structure
      : rule<create<xsmp::Structure> + visibility_prefix
             + kw<"struct">.doc<R"(`[visibility] struct <name> { ... }`

Defines a structured value type composed of fields.
Use a structure when several values belong together and need to be passed, stored or configured as one typed object.

**Example**
```xsmp
public struct ThermalLoop
{
    field Smp.Float64 temperature
    field Smp.Float64 pressure
}
```)">() + name
             + lit<"{">
             + many(
                 append<&xsmp::Structure::elements>(ref<Constant> | ref<Field>))
             + lit<"}">> {};

  // What a class and an exception share after their keyword. Each extends its
  // own kind.
  template <class Base, cursiv::str BaseDoc>
  static constexpr auto class_body =
      name
      + option(kw<"extends">.doc<BaseDoc>()
               + assign<xref<&xsmp::Class::base, Base>>(ref<QualifiedName>))
      + lit<"{">
      + many(append<&xsmp::Structure::elements>(ref<Constant> | ref<Field>
                                                | ref<Property> | ref<Operation>
                                                | ref<Association>))
      + lit<"}">;

  struct Class
      : rule<
            create<xsmp::Class> + class_prefix
            + kw<"class">.doc<R"(`[visibility] [abstract] class <name> [extends <base>] { ... }`

Defines a reusable implementation-oriented type with inheritance.
Classes extend structures with properties, operations and associations and are useful for shared implementation patterns.

**Example**
```xsmp
public abstract class BaseAlarm
{
    field Smp.String8 message
    def void acknowledge()
}
```)">() + class_body<xsmp::Class, "A single base class to extends from">> {};

  struct Exception
      : rule<
            create<xsmp::Exception> + class_prefix
            + kw<"exception">.doc<R"(`[visibility] [abstract] exception <name> [extends <base>] { ... }`

Defines an exception type that operations or properties may raise.
Use exception fields to carry diagnostic data that callers can inspect.

**Example**
```xsmp
public exception InvalidMode
{
    field Smp.String8 reason
}
```)">()
            + class_body<xsmp::Exception,
                         "A single base exception to extends from">> {};

  struct Interface
      : rule<
            create<xsmp::Interface> + visibility_prefix
            + kw<"interface">.doc<R"(`[visibility] interface <name> [extends <base>, ...] { ... }`

Defines a contract published or consumed by components.
Interfaces are the preferred way to model stable interaction points between otherwise independent components.

**Example**
```xsmp
public interface ICommandable
{
    property Mode mode
    def void apply(in Mode requestedMode)
}
```)">() + name
            + option(
                kw<"extends">.doc<"Interface(s) to inherit from">()
                + some(append<xref<&xsmp::Interface::base, xsmp::Interface>>(
                           ref<QualifiedName>),
                       lit<",">))
            + lit<"{">
            + many(append<&xsmp::Interface::elements>(
                ref<Constant> | ref<Property> | ref<Operation>))
            + lit<"}">> {};

  // What a model and a service share after their keyword. Each extends its own
  // kind.
  template <class Base, cursiv::str BaseDoc>
  static constexpr auto component_body =
      name
      + option(kw<"extends">.doc<BaseDoc>()
               + assign<xref<&xsmp::Component::base, Base>>(ref<QualifiedName>))
      + option(
          kw<"implements">.doc<"Interface(s) to inherit from">()
          + some(append<xref<&xsmp::Component::interface_, xsmp::Interface>>(
                     ref<QualifiedName>),
                 lit<",">))
      + lit<"{">
      + many(append<&xsmp::Component::elements>(
          ref<Constant> | ref<Field> | ref<Property> | ref<Operation>
          | ref<Association> | ref<Container> | ref<Reference> | ref<EntryPoint>
          | ref<EventSink> | ref<EventSource>))
      + lit<"}">;

  struct Model
      : rule<
            create<xsmp::Model> + class_prefix
            + kw<"model">.doc<R"(`[visibility] [abstract] model <name> [extends <base>] [implements <interface>, ...] { ... }`

Defines a simulation model.
Models can expose state, behavior, entry points, event endpoints, child containers and references to other components.

**Example**
```xsmp
public model AvionicsUnit implements demo.foundation.ICommandable
{
    field demo.foundation.Mode modeState
    public property demo.foundation.Mode mode -> modeState
    entrypoint step
}
```)">() + component_body<xsmp::Model, "A single base model to extends from">> {
  };

  struct Service
      : rule<
            create<xsmp::Service> + class_prefix
            + kw<"service">.doc<R"(`[visibility] [abstract] service <name> [extends <base>] [implements <interface>, ...] { ... }`

Defines a simulation service.
Services share the same component mechanisms as models but typically represent environment capabilities or infrastructure provided to models.

**Example**
```xsmp
public service EventManager implements Smp.IService
{
}
```)">()
            + component_body<xsmp::Service,
                             "A single base service to extends from">> {};

  struct Array
      : rule<
            visibility_prefix
            + kw<"array">.doc<R"(`[visibility] array <name> = <item-type>[<size>]`

Defines a fixed-size array type.
Arrays are value types, so they are useful for vectors, samples, lookup tables and other bounded collections.

**Example**
```xsmp
public array FloatSamples = Smp.Float32[16]
```)">() + name
            + lit<"=">
            + assign<xref<&xsmp::Array::itemType, xsmp::ValueType>>(
                ref<QualifiedName>)
            + lit<"["> + option(assign<&xsmp::Array::size>(ref<Expression>))
            + lit<"]">> {};

  struct ValueReference
      : rule<visibility_prefix
             + kw<"using">.doc<R"(`[visibility] using <name> = <value-type>*`

Defines a reference type to a value type.
Use it when a model API needs to point to a value object rather than contain it directly.

**Example**
```xsmp
public using VoltageRef = demo.foundation.Voltage*
```)">() + name
             + lit<"=">
             + assign<xref<&xsmp::ValueReference::type, xsmp::ValueType>>(
                 ref<QualifiedName>)
             + lit<"*">> {};

  struct Integer
      : rule<
            create<xsmp::Integer> + visibility_prefix
            + kw<"integer">.doc<R"(`[visibility] integer <name> [extends <primitive>] [in <min> ... <max>]`

Defines a constrained integer type.
Use `extends` to choose the storage primitive and `in` to express the accepted value range.

**Example**
```xsmp
public integer Percentage extends Smp.UInt8 in 0 ... 100
```)">() + name
            + option(kw<"extends">.doc<"The Integral type to inherit from">()
                     + assign<xref<&xsmp::Integer::primitiveType,
                                   xsmp::PrimitiveType>>(ref<QualifiedName>))
            + option(kw<"in">.doc<"The integer range">()
                     + (assign<&xsmp::Integer::minimum>(ref<Expression>)
                        | lit<"*">)+lit<"...">
                     + (assign<&xsmp::Integer::maximum>(ref<Expression>)
                        | lit<"*">))> {};

  struct Float
      : rule<
            create<xsmp::Float> + visibility_prefix
            + kw<"float">.doc<R"(`[visibility] float <name> [extends <primitive>] [in <min> <range-kind> <max>]`

Defines a constrained floating-point type.
Use the range kind to choose whether the bounds are inclusive or exclusive.

**Example**
```xsmp
public float TemperatureC extends Smp.Float32 in -120.0 ... 180.0
```)">() + name
            + option(kw<"extends">.doc<"The Floating type to inherit from">()
                     + assign<xref<&xsmp::Float::primitiveType,
                                   xsmp::PrimitiveType>>(ref<QualifiedName>))
            + option(kw<"in">.doc<"The integer range">()
                     + (assign<&xsmp::Float::minimum>(ref<Expression>)
                        | lit<"*">)+assign<&xsmp::Float::range>(range_kind)
                     + (assign<&xsmp::Float::maximum>(ref<Expression>)
                        | lit<"*">))> {};

  struct EventType
      : rule<
            create<xsmp::EventType> + visibility_prefix
            + kw<"event">.doc<R"(`[visibility] event <name> [extends <simple-type>]`

Defines an event type.
Extend a simple type when each emitted event carries one payload value of that type.

**Example**
```xsmp
public event ModeEvent extends demo.foundation.Mode
```)">() + name
            + option(
                kw<"extends">.doc<"The event argument type">()
                + assign<xref<&xsmp::EventType::eventArgs, xsmp::SimpleType>>(
                    ref<QualifiedName>))> {};

  struct String
      : rule<visibility_prefix
             + kw<"string">.doc<R"(`[visibility] string <name>[<length>]`

Defines a fixed-length string type based on `Char8`.
Use string types when the model needs bounded textual data with explicit maximum storage.

**Example**
```xsmp
public string Label[32]
```)">() + name
             + lit<"["> + option(assign<&xsmp::String::length>(ref<Expression>))
             + lit<"]">> {};

  struct PrimitiveType
      : rule<create<xsmp::PrimitiveType> + visibility_prefix
             + kw<"primitive">.doc<R"(`[visibility] primitive <name>`

Declares one primitive type entry.
Primitive types exist mainly to bootstrap the XSMP type system and are typically not authored in user catalogues.

**Example**
```xsmp
primitive Float64
```)">() + name> {};

  struct NativeType : rule<create<xsmp::NativeType> + visibility_prefix
                           + kw<"native">.doc<R"(`[visibility] native <name>`

Declares a native type placeholder used to bind XSMP models to platform-specific or external types.

**Example**
```xsmp
public native FILE_HANDLE
```)">() + name> {};

  struct AttributeType
      : rule<
            visibility_prefix
            + kw<"attribute">.doc<R"(`[visibility] attribute <type> <name> [= <default-value>]`

Declares a user-defined attribute type.
Attribute types let you attach structured metadata to XSMP elements in a controlled and typed way.

**Example**
```xsmp
public attribute Smp.String8 category = "telemetry"
```)">()
            + assign<xref<&xsmp::AttributeType::type, xsmp::ValueType>>(
                ref<QualifiedName>)
            + name
            + option(
                lit<"=">
                + assign<&xsmp::AttributeType::default_>(ref<Expression>))> {};

  struct EnumerationLiteral
      : rule<attributes + name + lit<"=">
             + assign<&xsmp::EnumerationLiteral::value>(ref<Expression>)> {};

  struct Enumeration
      : rule<
            create<xsmp::Enumeration> + visibility_prefix
            + kw<"enum">.doc<R"(`[visibility] enum <name> { <literal> = <value>, ... }`

Defines an enumeration type with named integer-backed literals.

**Example**
```xsmp
public enum Mode { Standby = 0, Nominal = 1, Safe = 2 }
```)">() + name
            + lit<"{">
            + option(some(append<&xsmp::Enumeration::literal>(
                              ref<EnumerationLiteral>),
                          lit<",">)
                     + option(lit<",">))
            + lit<"}">> {};

  struct Type
      : rule<ref<Structure> | ref<Class> | ref<Exception> | ref<Interface>
             | ref<Model> | ref<Service> | ref<Array> | ref<ValueReference>
             | ref<Integer> | ref<Float> | ref<EventType> | ref<String>
             | ref<PrimitiveType> | ref<NativeType> | ref<AttributeType>
             | ref<Enumeration>> {};

  // The members of a type.

  struct Constant
      : rule<
            visibility_prefix
            + kw<"constant">.doc<R"(`[visibility] constant <simple-type> <name> = <value>`

Declares a typed constant.
Constants are useful for protocol values, engineering constants and reusable defaults.

**Example**
```xsmp
public constant Smp.Float64 StandardGravity = 9.80665
```)">()
            + assign<xref<&xsmp::Constant::type, xsmp::SimpleType>>(
                ref<QualifiedName>)
            + name
            + option(lit<"=">
                     + assign<&xsmp::Constant::value>(ref<Expression>))> {};

  struct Field
      : rule<
            prefix_with<field_modifiers>
            + kw<"field">.doc<R"(`[visibility] [input|output|transient] field <type> <name> [= <default-value>]`

Declares state or data published by a type.
Use `input` and `output` for dataflow contracts, and `transient` when the value shall not be persisted as state.

**Example**
```xsmp
output field demo.foundation.Vector3 estimatedRates
```)">()
            + assign<xref<&xsmp::Field::type, xsmp::LanguageType>>(
                ref<QualifiedName>)
            + name
            + option(lit<"=">
                     + assign<&xsmp::Field::default_>(ref<Expression>))> {};

  struct Property
      : rule<
            prefix_with<property_modifiers>
            + kw<"property">.doc<R"(`[visibility] [readOnly|writeOnly|readWrite] property <type> <name> ...`

Declares a property exposed through getter/setter semantics instead of direct state access.
Use properties when access must be validated, transformed or published through interfaces.

**Example**
```xsmp
public property demo.foundation.Mode mode -> modeState
```)">()
            + assign<xref<&xsmp::Property::type, xsmp::LanguageType>>(
                ref<QualifiedName>)
            + name
            + option(kw<"get"> + kw<"throws">
                     + some(append<xref<&xsmp::Property::getRaises,
                                        xsmp::Exception>>(ref<QualifiedName>),
                            lit<",">))
            + option(kw<"set"> + kw<"throws">
                     + some(append<xref<&xsmp::Property::setRaises,
                                        xsmp::Exception>>(ref<QualifiedName>),
                            lit<",">))
            + option(
                lit<"->">
                + assign<&xsmp::Property::attachedField>(ref<QualifiedName>))> {
  };

  struct ReturnParameter
      : rule<many(append<&xsmp::ReturnParameter::attributes>(ref<Attribute>))
             + assign<xref<&xsmp::ReturnParameter::type, xsmp::LanguageType>>(
                 ref<QualifiedName>)
             + option(assign<&xsmp::ReturnParameter::name>(valid_id))> {};

  struct Parameter
      : rule<attributes
             + option(assign<&xsmp::Parameter::direction>(parameter_direction))
             + assign<xref<&xsmp::Parameter::type, xsmp::LanguageType>>(
                 ref<QualifiedName>)
             + name
             + option(lit<"=">
                      + assign<&xsmp::Parameter::default_>(ref<Expression>))> {
  };

  struct Operation
      : rule<
            create<xsmp::Operation> + visibility_prefix
            + kw<"def">.doc<R"(`[visibility] def (<return-type>|void) <name>(...) [throws <exception>, ...]`

Declares an operation callable through publication and dynamic invocation.
Operations may define ordered parameters, an optional return value and raised exceptions.

**Example**
```xsmp
public def void apply(in demo.foundation.Mode requestedMode)
```)">()
            + (kw<"void">
               | assign<&xsmp::Operation::returnParameter>(
                   ref<ReturnParameter>))
            + name + lit<"(">
            + many(append<&xsmp::Operation::parameter>(ref<Parameter>),
                   lit<",">)
            + lit<")">
            + option(kw<"throws">
                     + some(append<xref<&xsmp::Operation::raisedException,
                                        xsmp::Exception>>(ref<QualifiedName>),
                            lit<",">))> {};

  struct Association
      : rule<visibility_prefix
             + kw<"association">.doc<R"(`[visibility] association <type> <name>`

Declares a semantic association to another model element or value type.
Associations are useful when you want to express a relation without using component containment or reference mechanics.

**Example**
```xsmp
public association demo.foundation.Mode requestedMode
```)">()
             + assign<xref<&xsmp::Association::type, xsmp::LanguageType>>(
                 ref<QualifiedName>)
             + name> {};

  struct Container
      : rule<
            attributes
            + kw<"container">.doc<R"(`container <type>[<multiplicity>] <name> [= <default-component>]`

Multiplicity forms:
```
  ?
  *
  +
  [<value>]
  [<lower>...<upper>]
  [<lower>...*]
```

Declares a containment slot for child components.
Use multiplicity to constrain how many children may be placed in the container, and `= <default-component>` to suggest an initial implementation.

**Example**
```xsmp
container demo.avionics.SensorHead* sensors = demo.avionics.SensorHead
```)">()
            + assign<xref<&xsmp::Container::type, xsmp::ReferenceType>>(
                ref<QualifiedName>)
            + multiplicity_declaration + name
            + option(lit<"=">
                     + assign<xref<&xsmp::Container::defaultComponent,
                                   xsmp::Component>>(ref<QualifiedName>))> {};

  struct Reference
      : rule<attributes
             + kw<"reference">.doc<R"(`reference <type>[<multiplicity>] <name>`

Multiplicity forms:
```
  ?
  *
  +
  [<value>]
  [<lower>...<upper>]
  [<lower>...*]
```

Declares an aggregation reference to external components.
References are typed by a reference type, which can be an interface or a component depending on the selected SMP revision.

**Example**
```xsmp
reference demo.foundation.ILogger* loggers
```)">()
             + assign<xref<&xsmp::Reference::interface_, xsmp::ReferenceType>>(
                 ref<QualifiedName>)
             + multiplicity_declaration + name> {};

  struct EntryPoint
      : rule<create<xsmp::EntryPoint> + attributes
             + kw<"entrypoint">.doc<R"(`entrypoint <name> { in ... out ... }`

Declares a scheduler- or event-triggerable entry point on a component.
Use optional `in` and `out` sections to describe which published fields the entry point consumes or produces.

**Example**
```xsmp
entrypoint step
{
    out thermal
}
```)">() + name
             + option(
                 lit<"{">
                 + many((kw<"in">
                         + append<&xsmp::EntryPoint::input>(ref<QualifiedName>))
                        | (kw<"out">
                           + append<&xsmp::EntryPoint::output>(
                               ref<QualifiedName>)))
                 + lit<"}">)> {};

  struct EventSink
      : rule<attributes + kw<"eventsink">.doc<R"(`eventsink <event-type> <name>`

Declares an event input published by a component.

**Example**
```xsmp
eventsink demo.foundation.ModeEvent commandedMode
```)">()
             + assign<xref<&xsmp::EventSink::type, xsmp::EventType>>(
                 ref<QualifiedName>)
             + name> {};

  struct EventSource
      : rule<attributes
             + kw<"eventsource">.doc<R"(`eventsource <event-type> <name>`

Declares an event output published by a component.

**Example**
```xsmp
eventsource demo.foundation.ModeEvent publishedMode
```)">()
             + assign<xref<&xsmp::EventSource::type, xsmp::EventType>>(
                 ref<QualifiedName>)
             + name> {};

  struct Attribute
      : rule<lit<"@">
             + option(assign<xref<&xsmp::Attribute::type, xsmp::AttributeType>>(
                          ref<QualifiedName>)
                      + option(lit<"(">
                               + option(assign<&xsmp::Attribute::value>(
                                   ref<Expression>))
                               + lit<")">))> {};

  // What a namespace and a nested one share after the optional keyword.
  struct NestedNamespace;
  struct Namespace;
  static constexpr auto namespace_body =
      name
      + ((lit<"::"> + append<&xsmp::Namespace::elements>(ref<NestedNamespace>))
         | (lit<"{">
            + many(
                append<&xsmp::Namespace::elements>(ref<Namespace> | ref<Type>))
            + lit<"}">));

  struct Namespace
      : rule<
            attributes
            + kw<"namespace">.doc<R"(`namespace <name> [::<nested-name>]* { ... }`

Groups related types under a qualified name.
Namespaces may be nested and help keep large catalogues readable and collision-free.

**Example**
```xsmp
namespace demo::foundation
{
    public enum Mode { Standby = 0, Nominal = 1 }
}
```)">() + namespace_body> {};

  struct NestedNamespace : rule<attributes + namespace_body> {};

  struct Catalogue : rule<attributes + kw<"catalogue">.doc<R"(`catalogue <name>`

Declares a catalogue document.
A catalogue is the root container for XSMP type definitions and usually starts with one or more namespaces.

**Example**
```xsmp
catalogue foundation_catalogue
```)">() + name + many(append<&xsmp::Catalogue::elements>(ref<Namespace>))> {};

  using entry = Catalogue;
};

}  // namespace xsmp
