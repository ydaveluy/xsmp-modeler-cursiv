#pragma once

#include <cstdint>
#include <cstdlib>
#include <string>
#include <string_view>
#include <xsmp/CommonGrammar.hpp>
#include <xsmp/LiteralConversions.hpp>
#include <xsmp/ast.hpp>

namespace xsmp {

// The suffix belongs to the token but not to the text between the quotes.
inline constexpr auto deployment_time = [](std::string_view text) {
  return quoted_text(text.substr(0, text.find_last_of('"') + 1));
};
// A FLOAT may group its digits with `'`, which the conversion does not read.
inline constexpr auto deployment_real =
    [](std::string_view text) -> cursiv::ConversionResult<double> {
  if (text.find('\'') == std::string_view::npos) {
    auto result = cursiv::default_convert<double>(text);
    if (result) return result;
  }

  std::string normalized(text);
  std::erase(normalized, '\'');
  auto result = cursiv::default_convert<double>(normalized);
  if (result) return result;
  // A literal out of range reads as infinity or zero, not as a syntax error:
  // the validator reports it against its type.
  char* end = nullptr;
  const double value = std::strtod(normalized.c_str(), &end);
  if (end != normalized.data() && end == normalized.data() + normalized.size())
    return cursiv::ConversionResult<double>::ok(value);
  return result;
};

// What the four deployment languages share. Each one overrides what its syntax
// changes, such as `ValidID` for its own keywords.
struct DeploymentRules : CommonRules {
  static constexpr auto identifier = cr<"_a-zA-Z"> + many(w);
  static constexpr auto templated = lit<"{"> + identifier + lit<"}">;

  struct ID : terminal<identifier> {};

  // A name with at least one `{parameter}` placeholder.
  struct PATTERN : terminal<(identifier + some(templated + many(identifier)))
                            | (templated + many(identifier | templated))> {};

  struct STRING
      : terminal<as<quoted_text>(lit<"\""> + string_text + lit<"\"">)> {};

  struct CHAR : terminal<as<quoted_text>(lit<"'"> + char_text + lit<"'">)> {};

  // For a text that may mean more than it spells. A date, a duration or a
  // template value has no valid form with an escape, so it stays as written.
  struct DecodedString : rule<as<decoded_text>(ref<STRING>)> {};

  struct DecodedChar : rule<as<decoded_text>(ref<CHAR>)> {};

  // The suffix gives the type: `"PT1S"d` is a duration, and
  // `"1970-01-01T00:00:00Z"dt` a date and time.
  struct DURATION_STRING
      : terminal<as<deployment_time>(lit<"\""> + string_text + lit<"\"d">)> {};

  struct DATETIME_STRING
      : terminal<as<deployment_time>(lit<"\""> + string_text + lit<"\"dt">)> {};

  static constexpr auto unsigned_integer = lit<"0"> | (cr<"1-9"> + many(d));

  struct INT : terminal<option(cr<"+-">) + unsigned_integer> {};

  // Digits may be grouped with `'`, as in `1'000.5`.
  static constexpr auto digits = d + many(cr<"0-9'">);
  static constexpr auto exponent = cr<"eE"> + option(cr<"+-">) + digits;

  struct FLOAT
      : terminal<as<deployment_real>(
            option(cr<"+-">)
            + ((((option(digits) + lit<"."> + digits) | (digits + lit<".">))
                + option(exponent))
               | (digits + exponent)))> {};

  // A name. A language adds the keywords it also accepts as names.
  struct ValidID : rule<ref<ID>> {};

  // A word that also stands as a keyword where a name is read gives way to it,
  // unless what follows can only follow a name.
  static constexpr auto name_goes_on =
      &(lit<"="> | lit<"+="> | lit<"."> | lit<"["> | lit<"{"> | lit<":">
        | lit<"->"> | lit<"(">);

  // A name where `unsafe` may stand before it.
  struct LocalName : rule<(kw<"unsafe"> + name_goes_on) | ref<ValidID>> {};

  static constexpr auto more_names = many(lit<"."> + ref<ValidID>);

  struct QualifiedName : rule<ref<ValidID> + more_names> {};

  struct SignedInteger : rule<as<std::int64_t>(ref<INT>)> {};

  struct UnsignedInteger : rule<as<std::uint64_t>(ref<INT>)> {};

  // A dot never starts `..`, in paths where both can stand.
  static constexpr auto path_dot = lit<".", !lit<".">>;

  struct Path;
  struct PathSegment;
  struct PathElement;
  struct Value;
  struct ArrayValue;
  struct StructureValue;

  // A local name reads a member of the current Component, and each use accepts
  // one kind of member.
  template <class Target>
  static constexpr auto local_reference =
      option(flag<&xsmp::LocalNamedReference::unsafe>(kw<"unsafe">))
      + assign<xref<&xsmp::LocalNamedReference::reference, Target>>(
          ref<LocalName>);

  // A local name of a `Reference` member, not any local name.
  struct LocalReference : rule<local_reference<xsmp::Reference>> {};

  struct ConcretePathNamedSegment
      : rule<assign<&xsmp::ConcretePathNamedSegment::reference>(ref<ValidID>)> {
  };

  struct ConcretePathHead
      : rule<assign<&xsmp::ConcretePathNamedSegment::reference>(
            ref<LocalName>)> {};

  struct PatternPathNamedSegment
      : rule<assign<&xsmp::PatternPathNamedSegment::text>(ref<PATTERN>)> {};

  struct PathParentSegment : rule<create<xsmp::PathParentSegment> + lit<"..">> {
  };

  struct PathSelfSegment : rule<create<xsmp::PathSelfSegment> + path_dot> {};

  struct PathSegment
      : rule<ref<ConcretePathNamedSegment> | ref<PatternPathNamedSegment>
             | ref<PathParentSegment> | ref<PathSelfSegment>> {};

  struct PathHead : rule<ref<ConcretePathHead> | ref<PatternPathNamedSegment>
                         | ref<PathParentSegment> | ref<PathSelfSegment>> {};

  struct PathMember
      : rule<assign<&xsmp::PathMember::separator>(path_dot | lit<"/">)
             + assign<&xsmp::PathMember::segment>(ref<PathSegment>)> {};

  struct PathIndex
      : rule<lit<"["> + assign<&xsmp::PathIndex::index>(ref<SignedInteger>)
             + lit<"]">> {};

  struct PathElement : rule<ref<PathMember> | ref<PathIndex>> {};

  static constexpr auto path_elements =
      many(append<&xsmp::Path::elements>(ref<PathElement>));

  // `[unsafe] [/] head (.member | /member | [index])*`. The `/` may stand
  // alone, as the root.
  struct Path
      : rule<create<xsmp::Path>
             + option(flag<&xsmp::Path::unsafe>(kw<"unsafe">))
             + ((flag<&xsmp::Path::absolute>(lit<"/">)
                 + option(assign<&xsmp::Path::head>(ref<PathSegment>)
                          + path_elements))
                | (assign<&xsmp::Path::head>(ref<PathHead>) + path_elements))> {
  };

  // A path with a keyword as its first name, read by `Word`.
  template <class Word>
  static constexpr auto word_path =
      create<xsmp::Path> + assign<&xsmp::Path::head>(ref<Word>) + path_elements;

  // A path that starts a statement. A language adds the words that also start
  // one, which stay names with `word_path`.
  struct StatementPath : rule<ref<Path>> {};

  // The suffix of a number names its SMP type. Without one, the number takes
  // its type from where it is used.
  struct Int8Value
      : rule<assign<&xsmp::Int8Value::value>(ref<SignedInteger>) + kw<"i8">> {};

  struct Int16Value
      : rule<assign<&xsmp::Int16Value::value>(ref<SignedInteger>) + kw<"i16">> {
  };

  struct Int32Value
      : rule<assign<&xsmp::Int32Value::value>(ref<SignedInteger>) + kw<"i32">> {
  };

  struct Int64Value
      : rule<assign<&xsmp::Int64Value::value>(ref<SignedInteger>) + kw<"i64">> {
  };

  struct UInt8Value
      : rule<assign<&xsmp::UInt8Value::value>(ref<UnsignedInteger>)
             + kw<"u8">> {};

  struct UInt16Value
      : rule<assign<&xsmp::UInt16Value::value>(ref<UnsignedInteger>)
             + kw<"u16">> {};

  struct UInt32Value
      : rule<assign<&xsmp::UInt32Value::value>(ref<UnsignedInteger>)
             + kw<"u32">> {};

  struct UInt64Value
      : rule<assign<&xsmp::UInt64Value::value>(ref<UnsignedInteger>)
             + kw<"u64">> {};

  struct IntValue : rule<assign<&xsmp::IntValue::value>(ref<INT>)> {};

  // A literal by name, or by number: `2 lit`.
  struct EnumerationValue
      : rule<(assign<&xsmp::EnumerationValue::value>(ref<SignedInteger>)
              + kw<"lit">)
             | assign<&xsmp::EnumerationValue::reference>(ref<QualifiedName>)> {
  };

  struct BoolValue
      : rule<create<xsmp::BoolValue>
             + (flag<&xsmp::BoolValue::value>(kw<"true">) | kw<"false">)> {};

  struct Float32Value
      : rule<assign<&xsmp::Float32Value::value>(ref<FLOAT>) + kw<"f32">> {};

  struct Float64Value
      : rule<assign<&xsmp::Float64Value::value>(ref<FLOAT>) + kw<"f64">> {};

  struct FloatValue : rule<assign<&xsmp::FloatValue::value>(ref<FLOAT>)> {};

  struct Char8Value : rule<assign<&xsmp::Char8Value::value>(ref<DecodedChar>)> {};

  struct String8Value : rule<assign<&xsmp::String8Value::value>(ref<DecodedString>)> {
  };

  struct DurationValue
      : rule<assign<&xsmp::DurationValue::value>(ref<DURATION_STRING>)> {};

  struct DateTimeValue
      : rule<assign<&xsmp::DateTimeValue::value>(ref<DATETIME_STRING>)> {};

  struct SimpleValue
      : rule<ref<Int8Value> | ref<Int16Value> | ref<Int32Value>
             | ref<Int64Value> | ref<UInt8Value> | ref<UInt16Value>
             | ref<UInt32Value> | ref<UInt64Value> | ref<BoolValue>
             | ref<Float32Value> | ref<Float64Value> | ref<FloatValue>
             | ref<IntValue> | ref<Char8Value> | ref<String8Value>
             | ref<EnumerationValue> | ref<DurationValue>
             | ref<DateTimeValue>> {};

  struct FieldValue
      : rule<assign<&xsmp::FieldValue::field>(ref<StatementPath>) + lit<"=">
             + assign<&xsmp::FieldValue::value>(ref<Value>)> {};

  // In a structure value, a value with no field takes the next one still free.
  struct OptFieldValue
      : rule<option(assign<&xsmp::FieldValue::field>(ref<Path>) + lit<"=">)
             + assign<&xsmp::FieldValue::value>(ref<Value>)> {};

  // `[3: a, b]` starts at index 3.
  struct ArrayValue
      : rule<create<xsmp::ArrayValue> + lit<"[">
             + option(assign<&xsmp::ArrayValue::startIndex>(ref<SignedInteger>)
                      + lit<":">)
             + many(append<&xsmp::ArrayValue::elements>(ref<Value>), lit<",">)
             + lit<"]">> {};

  struct StructureValue
      : rule<create<xsmp::StructureValue> + lit<"{">
             + many(append<&xsmp::StructureValue::elements>(ref<OptFieldValue>),
                    lit<",">)
             + lit<"}">> {};

  struct Value
      : rule<ref<SimpleValue> | ref<ArrayValue> | ref<StructureValue>> {};

  // A template parameter states its type, its default value, or both.
  struct StringParameter
      : rule<assign<&xsmp::TemplateParameter::name>(ref<ID>)
             + ((lit<":"> + kw<"string">.doc<R"(`string`

Declares a string template parameter.
String parameters are used inside templated names and paths.

**Example**
```xsmp
Lane: string = "Ops"
```)">()
                 + option(lit<"=">
                          + assign<&xsmp::StringParameter::value>(ref<STRING>)))
                | (lit<"=">
                   + assign<&xsmp::StringParameter::value>(ref<STRING>)))
             + create<xsmp::StringParameter>> {};

  struct Int32Parameter
      : rule<assign<&xsmp::TemplateParameter::name>(ref<ID>)
             + ((lit<":"> + kw<"int32">.doc<R"(`int32`

Declares an `int32` template parameter.

**Example**
```xsmp
Index: int32 = 1
```)">()
                 + option(lit<"=">
                          + assign<&xsmp::Int32Parameter::value>(
                              ref<SignedInteger>)))
                | (lit<"=">
                   + assign<&xsmp::Int32Parameter::value>(ref<SignedInteger>)))
             + create<xsmp::Int32Parameter>> {};

  struct TemplateParameter : rule<ref<StringParameter> | ref<Int32Parameter>> {
  };

  struct StringArgument
      : rule<assign<&xsmp::TemplateArgument::parameter>(ref<ValidID>) + lit<"=">
             + assign<&xsmp::StringArgument::value>(ref<STRING>)> {};

  struct Int32Argument
      : rule<assign<&xsmp::TemplateArgument::parameter>(ref<ValidID>) + lit<"=">
             + assign<&xsmp::Int32Argument::value>(ref<SignedInteger>)> {};

  struct TemplateArgument : rule<ref<StringArgument> | ref<Int32Argument>> {};

  // `name = value` in a call. Validation finds the parameter from the name.
  struct ParameterValue
      : rule<assign<&xsmp::ParameterValue::parameter>(ref<ValidID>) + lit<"=">
             + assign<&xsmp::ParameterValue::value>(ref<SimpleValue>)> {};

  struct EventLink : rule<create<xsmp::EventLink>
                          + kw<"event">.doc<R"(`event link <owner> -> <client>`

Connects an event source to a compatible event sink.

**Example**
```xsmp
event link publishedMode -> child.commandedMode
```)">() + kw<"link"> + assign<&xsmp::Link::ownerPath>(ref<Path>)
                          + lit<"->">
                          + assign<&xsmp::Link::clientPath>(ref<Path>)> {};

  struct FieldLink : rule<create<xsmp::FieldLink>
                          + kw<"field">.doc<R"(`field link <owner> -> <client>`

Connects an output field to an input field.

**Example**
```xsmp
field link outValue -> child.inValue
```)">() + kw<"link"> + assign<&xsmp::Link::ownerPath>(ref<Path>)
                          + lit<"->">
                          + assign<&xsmp::Link::clientPath>(ref<Path>)> {};

  struct InterfaceLink
      : rule<
            create<xsmp::InterfaceLink>
            + kw<"interface">.doc<R"(`interface link <sourcePath> -> <client> [:<backReference>]`

Connects one published reference to a compatible client component.
The last segment of `sourcePath` selects the published reference, while any preceding segments navigate to the owner component instance.

**Example**
```xsmp
interface link logger -> payload
interface link nested.logger -> payload:backLogger
```)">() + kw<"link">
            + assign<&xsmp::InterfaceLink::sourcePath>(ref<Path>) + lit<"->">
            + assign<&xsmp::Link::clientPath>(ref<Path>)
            + option(lit<":">
                     + assign<&xsmp::InterfaceLink::backReference>(
                         ref<LocalReference>))> {};

  struct Link : rule<ref<EventLink> | ref<FieldLink> | ref<InterfaceLink>> {};
};

}  // namespace xsmp
