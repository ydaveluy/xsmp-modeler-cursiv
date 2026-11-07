#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>
#include <xsmp/ast.hpp>
#include <xsmp/references/QualifiedName.hpp>
#include <xsmp/utils/TreeUtils.hpp>

namespace xsmp {

/// True when `base` is `parent` or reaches it through its own `extends` chain
/// (used to detect a cyclic `extends` on a class/exception).
[[nodiscard]] bool isBaseOfClass(const Class* parent, const Type* base);

/// True when `base` is `parent` or reaches it through its own `extends` chain
/// (cyclic `extends` on a model/service).
[[nodiscard]] bool isBaseOfComponent(const Component* parent, const Type* base);

/// True when `base` is `parent` or reaches it through the interfaces it
/// extends (cyclic `extends` on an interface).
[[nodiscard]] bool isBaseOfInterface(const Interface* parent, const Type* base);

/// True when `base` is `parent` or reaches it through the interfaces it extends
/// or implements or its base component — used to check that a Container's
/// `defaultComponent` is a sub type of its `type`. `parent` shall be an
/// Interface or Component (a "ReferenceType"); `parentQualifiedName` is its
/// fully qualified name, needed for the built-in-interface implications SMP
/// infers from a Component's kind/structural elements (`Smp.IObject`,
/// `Smp.IComponent`, `Smp.IModel`, `Smp.IService`, `Smp.IEntryPointPublisher`,
/// `Smp.IComposite`, `Smp.IAggregate`, `Smp.IEventConsumer`,
/// `Smp.IEventProvider`, `Smp.IDynamicInvocation`).
[[nodiscard]] bool isBaseOfReferenceType(const Type* parent,
                                         std::string_view parentQualifiedName,
                                         const Type* base);

/// True when `other` recursively contains `parent` by value: `other` is
/// `parent` itself, an Array whose item type recursively contains it, or a
/// Structure (Class/Exception included) with a Field or a base that recursively
/// contains it. Used to reject a Field/Array whose type embeds its own
/// enclosing Structure/Array by value (infinite size).
[[nodiscard]] bool isRecursiveType(const Type* parent, const Type* other);

/// True when `text` is a canonical `8-4-4-4-12` hexadecimal UUID (no version/
/// variant constraint).
[[nodiscard]] bool isUUID(std::string_view text) noexcept;

/// Whether a name is an ISO/ANSI C++ keyword, which ECSS-E-ST-40-07 5.2.1 does
/// not allow for an SMP object.
[[nodiscard]] bool isCppKeyword(std::string_view name);

/// The effective lower multiplicity bound: `0` when `optional`, `1` when no
/// multiplicity is given, `aux ? 1 : 0` for a bare `[]`/`[+]`/`[*]`, else
/// the solved `lower` expression. `std::nullopt` when `lower` is present but
/// not a compile-time constant.
[[nodiscard]] std::optional<std::int64_t> getLower(
    const NamedElementWithMultiplicity& element);

/// The effective upper multiplicity bound (`-1` meaning unbounded), with the
/// same cases as `getLower`. A single bound `[n]` gives both.
[[nodiscard]] std::optional<std::int64_t> getUpper(
    const NamedElementWithMultiplicity& element);

/// Needed wherever the THREE-way distinction matters (e.g. the generator's
/// per-member `public:`/`protected:`/`private:` access specifier tracking).
enum class Visibility : std::uint8_t { Public, Protected, Private };

/// Members of an EXACT (not subtype) Structure or Interface container are
/// always public regardless of any declared modifier; elsewhere an explicit
/// `private`/`protected`/`public` modifier wins, defaulting to `private` when
/// none is given. A node that isn't a `VisibilityElement` at all (e.g. a
/// `Container` or an `EntryPoint`) is always public.
[[nodiscard]] Visibility getRealVisibility(const NamedElement& node);

/// True when `node`'s EFFECTIVE visibility is `public`.
[[nodiscard]] bool isEffectivelyPublic(const NamedElement& node);

/// True when `node`'s effective visibility is private. `protected` counts as
/// visible here.
[[nodiscard]] bool isEffectivelyPrivate(const NamedElement& node);

/// True when `element`'s Constant is visible from `from`: referenced from
/// within the SAME enclosing Type (an in-progress declaration may reference its
/// own not-yet-fully-linked siblings), from a Type derived from it when it is
/// protected, or effectively public elsewhere.
[[nodiscard]] bool isConstantVisibleFrom(const Expression& from,
                                         const Constant& element);

/// True when `from` may reference `element`: a `protected` Type is visible only
/// from within the SAME document; a `private` Type only from within its
/// enclosing Namespace (or a nested scope thereof); a `public` Type (or a Type
/// not directly nested in a Namespace) is always visible.
[[nodiscard]] bool isTypeVisibleFrom(const cursiv::AstNode& from,
                                     const Type& element);

// --- modifier predicates -----------------------------------------------------
[[nodiscard]] constexpr bool isVisibilityModifier(std::string_view m) noexcept {
  return m == "private" || m == "protected" || m == "public";
}

[[nodiscard]] constexpr bool isAccessKind(std::string_view m) noexcept {
  return m == "readWrite" || m == "readOnly" || m == "writeOnly";
}

[[nodiscard]] constexpr bool isAbstractModifier(std::string_view m) noexcept {
  return m == "abstract";
}

[[nodiscard]] constexpr bool isInputModifier(std::string_view m) noexcept {
  return m == "input";
}

[[nodiscard]] constexpr bool isOutputModifier(std::string_view m) noexcept {
  return m == "output";
}

[[nodiscard]] constexpr bool isTransientModifier(std::string_view m) noexcept {
  return m == "transient";
}

// Well-known @Attributes.* usage helpers, shared by the validator and the C++
// generator. See `xsmp::AttributeHelper` for the set only code generation
// needs.

/// The `@id`-usage on `element` whose resolved AttributeType's fully-qualified
/// name is `id` (e.g. "Attributes.Abstract"), or nullptr.
template <typename T>
[[nodiscard]] const Attribute* findAttribute(const T& element,
                                             std::string_view id) {
  for (const auto* attribute : element.attributes) {
    if (attribute == nullptr) {
      continue;
    }
    if (const auto* type = attribute->type.get();
        type != nullptr && hasQualifiedName(*type, id)) {
      return attribute;
    }
  }
  return nullptr;
}

/// Resolves a well-known attribute usage NODE to a bool: its own `value`
/// expression if written, else the AttributeType's own `default_` expression.
/// `nullopt` when `attribute` is null or not a compile-time bool constant.
[[nodiscard]] std::optional<bool> attributeBoolValueOf(
    const Attribute* attribute);

/// Combines `findAttribute` and `attributeBoolValueOf`.
template <typename T>
[[nodiscard]] std::optional<bool> attributeBoolValue(const T& element,
                                                     std::string_view id) {
  return attributeBoolValueOf(findAttribute(element, id));
}

// `Attributes.Constructor` hides `Attributes.Static`, which hides
// `Attributes.Abstract`.
[[nodiscard]] bool isConstructorAttribute(const NamedElement& element);

[[nodiscard]] bool isStaticAttribute(const NamedElement& element);

/// Only called on members of a Component or Class, never of an Interface.
[[nodiscard]] bool isAbstractAttribute(const NamedElement& element);

/// The `@Attributes.SimpleArray` well-known attribute usage.
[[nodiscard]] bool isSimpleArrayAttribute(const NamedElement& element);

/// True when `type` is exactly the builtin `Smp.String8`.
[[nodiscard]] bool isString8(const Type* type);

// The qualifiers below say how the generated C++ writes an element. They depend
// on the kind of element, hence the overloads. Each applies its own default
// when no `@Attributes.*` attribute is written.

// `@Attributes.Const`. Without it, a Parameter is const when its direction is
// `in` or unset and its type is not a ValueType, and nothing else is const. A
// Parameter, a ReturnParameter or an Operation marked `@Attributes.Constructor`
// is never const.
[[nodiscard]] bool isConst(const Parameter& element);
[[nodiscard]] bool isConst(const ReturnParameter& element);
[[nodiscard]] bool isConst(const Association& element);
[[nodiscard]] bool isConst(const Property& element);
[[nodiscard]] bool isConst(const Operation& element);

// `@Attributes.ByPointer`. Without it, a Parameter is a pointer when its
// direction is `out` or `inout` and its type is a Component, an Interface, a
// NativeType or a ValueType; the other elements are pointers when their type
// is a Component or an Interface. Except on an Association,
// `@Attributes.ByReference` turns that default off.
/// What tells overloads apart: the name of `member` and, for an Operation, the
/// type, const, pointer and reference qualifiers of each parameter.
[[nodiscard]] std::string getSignature(const NamedElement& member);

[[nodiscard]] bool isByPointer(const Parameter& element);
[[nodiscard]] bool isByPointer(const ReturnParameter& element);
[[nodiscard]] bool isByPointer(const Association& element);
[[nodiscard]] bool isByPointer(const Property& element);

// `@Attributes.ByReference`. Without it, only a Parameter can be a reference:
// when its direction is `in` or unset, its type is a Component or an Interface
// and `@Attributes.ByPointer` does not say otherwise.
[[nodiscard]] bool isByReference(const Parameter& element);
[[nodiscard]] bool isByReference(const ReturnParameter& element);
[[nodiscard]] bool isByReference(const Property& element);

/// Whether the C++ name of `type` itself stands for a pointer, so that a value
/// of it can be `nullptr`: a ValueReference is a `T*`. A NativeType counts too:
/// it is the platform's to define, and only the C++ compiler can tell. A
/// `Smp.String8` is a `const char*` but stands for a text, as in the other XSMP
/// tools: it is no pointer here.
[[nodiscard]] bool isPointerType(const Type* type);

/// Whether the generated C++ type of `parameter` is a pointer: passed by
/// pointer, or by value with a pointer type. A C++ reference is not one.
[[nodiscard]] bool isPointer(const Parameter& parameter);

/// `text` as it stands between the `delimiter`s of a C++ string or character
/// literal, reading back as the same bytes. A byte takes its short escape if it
/// has one, stays as it is in a UTF-8 character, and is otherwise written in
/// three-digit octal, which never reads into what follows: `\x` would. A NUL is
/// `\0` unless a digit follows. An XSMP literal reads the same.
[[nodiscard]] std::string cppEscaped(std::string_view text,
                                     char delimiter = '"');

/// The non-static, effectively-public Fields of `structure`, base chain first
/// (recursively) then `structure`'s own: the fields a collection literal
/// addresses by position. `visited` guards against a cyclic `extends` chain,
/// which is still a valid tree before validation reports it.
[[nodiscard]] std::vector<const Field*> getAllFields(
    const Structure& structure, std::unordered_set<const Structure*>& visited);

/// Convenience overload of `getAllFields` that owns its own `visited` set —
/// for call sites that don't need to share/inspect it across calls.
[[nodiscard]] std::vector<const Field*> getAllFields(
    const Structure& structure);

}  // namespace xsmp
