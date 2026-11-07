#pragma once

#include <cstdint>
#include <optional>
#include <string_view>
#include <xsmp/ast.hpp>
#include <xsmp/references/QualifiedName.hpp>
#include <xsmp/utils/XsmpUtils.hpp>

// What the `@Attributes.*` attributes and the modifiers of a catalogue element
// mean for the generated C++, with the default that applies when none is
// written. `XsmpUtils` has the part that the validators need too, such as
// `isByPointer`.

namespace xsmp {

/// The `@Attributes.Mutable` attribute.
[[nodiscard]] bool isMutable(const NamedElement& element);

/// The name, without its `VK_` prefix, of the `Smp.ViewKind` literal given by
/// the `@Attributes.View` attribute: "None", "Debug", "Expert" or "All". The
/// attribute value is either a literal (`ViewKind.VK_Debug`) or a bare integer
/// (`1`). `std::nullopt` when the attribute is absent or not a constant.
[[nodiscard]] std::optional<std::string_view> getViewKind(
    const NamedElement& element);

/// The name of the `Attributes.Operator` literal on `op` (for example "Add"),
/// or an empty string when `op` has no constant `@Attributes.Operator`.
[[nodiscard]] std::string_view operatorLiteral(const Operation& op);

/// `false` when static; `true` when directly nested in an Interface; else the
/// `@Attributes.Abstract` attribute. For an Operation or a Property.
[[nodiscard]] bool isAbstract(const NamedElement& element);

/// `false` for a constructor; `true` when abstract; else the
/// `@Attributes.Virtual` attribute, which defaults to `true` for a non-static
/// member of an Interface or a Component and to `false` in a Class. For an
/// Operation or a Property.
[[nodiscard]] bool isVirtual(const NamedElement& element);

/// A Property's `readOnly`, `readWrite` or `writeOnly` modifier.
enum class AccessKind : std::uint8_t { ReadOnly, ReadWrite, WriteOnly };

/// `element`'s access modifier, or `std::nullopt` when it has none: such a
/// Property is read-write.
[[nodiscard]] std::optional<AccessKind> getAccessKind(const Property& element);

/// The `input` modifier.
[[nodiscard]] bool isInput(const VisibilityElement& element);
/// The `output` modifier.
[[nodiscard]] bool isOutput(const VisibilityElement& element);
/// True unless the element has the `transient` modifier.
[[nodiscard]] bool isState(const VisibilityElement& element);

/// The `abstract` modifier of a type, as opposed to the `@Attributes.Abstract`
/// of an Operation or a Property.
[[nodiscard]] bool isAbstractType(const VisibilityElement& element);

// `@Attributes.ConstGetter`: the getter of the Property is a const member.
[[nodiscard]] bool isConstGetter(const Property& element);

}  // namespace xsmp
