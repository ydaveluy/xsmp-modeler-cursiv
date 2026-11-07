// Copyright line one
// Copyright line two
// -----------------------------------------------------------------------------
// File Name    : Pair.h
// -----------------------------------------------------------------------------
/// @file top/inner/Pair.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef TOP_INNER_PAIR_H_
#define TOP_INNER_PAIR_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <Smp/ViewKind.h>
#include <TasMdk/Types/Structure.h>

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
/// A structure with designated initializers.
struct Pair
{
    ::Smp::Int32 x;
    ::Smp::Int32 y;

    static void _Register(::Smp::Publication::ITypeRegistry* registry);
};

/// Universally unique identifier of type Pair.
constexpr ::Smp::Uuid Uuid_Pair { 0x10000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000027U };

template<bool ...Opts>
struct _Pair : public ::TasMdk::Types::StructureType<Opts...>
{
    // the equivalent raw_type
    using raw_type = ::top::inner::Pair;

    // constructor
    _Pair(const std::string& name, const std::string& description,
            Smp::IObject *parent, Smp::ViewKind view,
            const Smp::Publication::ITypeRegistry *type_registry, Smp::Uuid typeUuid,
            const raw_type &default_value = raw_type{}) :
            ::TasMdk::Types::StructureType<Opts...>(name, description, parent, view, type_registry->GetType(typeUuid)),
            /// x initialization
            x{"x", "", this, view, type_registry, ::Smp::Uuids::Uuid_Int32, default_value.x},
            /// y initialization
            y{"y", "", this, view, type_registry, ::Smp::Uuids::Uuid_Int32, default_value.y} {
    }

    // copy operator
    _Pair & operator=(const _Pair &other)
    {
        x = other.x;
        y = other.y;
        return *this;
    }

    // copy operator from raw_type
    _Pair & operator=(const raw_type &other)
    {
        x = other.x;
        y = other.y;
        return *this;
    }

    // convert to raw_type
    operator raw_type() const noexcept
    {
        return {x, y};
    }
    typename ::TasMdk::SimpleField<::Smp::Int32>::in<Opts...>::type x;
    typename ::TasMdk::SimpleField<::Smp::Int32>::in<Opts...>::type y;
};
} // namespace inner
} // namespace top

#endif // TOP_INNER_PAIR_H_
