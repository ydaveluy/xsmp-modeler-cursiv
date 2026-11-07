// Copyright line one
// Copyright line two
// -----------------------------------------------------------------------------
// File Name    : Nested.h
// -----------------------------------------------------------------------------
/// @file top/inner/Nested.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef TOP_INNER_NESTED_H_
#define TOP_INNER_NESTED_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <Smp/ViewKind.h>
#include <TasMdk/Types/Structure.h>
#include <top/inner/Payload.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

/// Top namespace doc.
namespace top
{
/// Inner namespace doc.
///  Second line.
namespace inner
{
struct Nested
{
    ::top::inner::Payload inner;
    ::Smp::Bool ok;

    static void _Register(::Smp::Publication::ITypeRegistry* registry);
};

/// Universally unique identifier of type Nested.
constexpr ::Smp::Uuid Uuid_Nested { 0x10000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000015U };

template<bool ...Opts>
struct _Nested : public ::TasMdk::Types::StructureType<Opts...>
{
    // the equivalent raw_type
    using raw_type = ::top::inner::Nested;

    // constructor
    _Nested(const std::string& name, const std::string& description,
            Smp::IObject *parent, Smp::ViewKind view,
            const Smp::Publication::ITypeRegistry *type_registry, Smp::Uuid typeUuid,
            const raw_type &default_value = raw_type{}) :
            ::TasMdk::Types::StructureType<Opts...>(name, description, parent, view, type_registry->GetType(typeUuid)),
            /// inner initialization
            inner{"inner", "", this, view, type_registry, ::top::inner::Uuid_Payload, default_value.inner},
            /// ok initialization
            ok{"ok", "", this, view, type_registry, ::Smp::Uuids::Uuid_Bool, default_value.ok} {
    }

    // copy operator
    _Nested & operator=(const _Nested &other)
    {
        inner = other.inner;
        ok = other.ok;
        return *this;
    }

    // copy operator from raw_type
    _Nested & operator=(const raw_type &other)
    {
        inner = other.inner;
        ok = other.ok;
        return *this;
    }

    // convert to raw_type
    operator raw_type() const noexcept
    {
        return {inner, ok};
    }
    typename ::TasMdk::StructureField<::top::inner::_Payload>::in<Opts...>::type inner;
    typename ::TasMdk::SimpleField<::Smp::Bool>::in<Opts...>::type ok;
};
} // namespace inner
} // namespace top

#endif // TOP_INNER_NESTED_H_
