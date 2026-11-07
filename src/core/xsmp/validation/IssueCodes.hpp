#pragma once

#include <cursiv/services/JsonValue.hpp>
#include <string_view>

namespace xsmp {

// The `data` of a diagnostic with an issue code: a stable name for the issue,
// by which `XsmpCodeActionProvider` picks a repair.
inline cursiv::JsonValue diagnosticData(std::string_view code) {
  return cursiv::JsonValue::Object{{"code", code}};
}

inline constexpr std::string_view TypeNotVisible = "type-not-visible";
inline constexpr std::string_view FieldNotVisible = "field-not-visible";
inline constexpr std::string_view InvalidFieldName = "invalid-field-name";
inline constexpr std::string_view MissingValue = "missing-value";
inline constexpr std::string_view MissingUuid = "missing-uuid";
inline constexpr std::string_view DuplicatedUuid = "duplicated-uuid";
inline constexpr std::string_view DuplicatedTypeName = "duplicated-type-name";
inline constexpr std::string_view InvalidUuid = "invalid-uuid";
inline constexpr std::string_view IllegalModifier = "illegal-modifier";
inline constexpr std::string_view InvalidModifier = "invalid-modifier";
inline constexpr std::string_view DuplicatedInterfaceBase =
    "duplicated-interface-base";
inline constexpr std::string_view CyclicInterfaceBase = "cyclic-interface-base";
inline constexpr std::string_view CyclicComponentBase = "cyclic-component-base";
inline constexpr std::string_view CyclicClassBase = "cyclic-class-base";
inline constexpr std::string_view MissingAbstract = "missing-abstract";
inline constexpr std::string_view DuplicatedComponentInterface =
    "duplicated-component-interface";
inline constexpr std::string_view InvalidUsage = "invalid-usage";
inline constexpr std::string_view DuplicatedUsage = "duplicated-usage";
inline constexpr std::string_view NonSimpleArray = "non-simple-array";
inline constexpr std::string_view InvalidAttribute = "invalid-attribute";
inline constexpr std::string_view DuplicatedException = "duplicated-exception";
inline constexpr std::string_view EnumLiteralExpected = "not-enum-literal";

}  // namespace xsmp
