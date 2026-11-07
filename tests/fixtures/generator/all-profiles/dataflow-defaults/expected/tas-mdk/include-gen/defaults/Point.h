// -----------------------------------------------------------------------------
// File Name    : Point.h
// -----------------------------------------------------------------------------
/// @file defaults/Point.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef DEFAULTS_POINT_H_
#define DEFAULTS_POINT_H_

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

namespace defaults
{
struct Point
{
    ::Smp::Int32 x;
    ::Smp::Int32 y;

    static void _Register(::Smp::Publication::ITypeRegistry* registry);
};

/// Universally unique identifier of type Point.
constexpr ::Smp::Uuid Uuid_Point { 0x70000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000005U };

template<bool ...Opts>
struct _Point : public ::TasMdk::Types::StructureType<Opts...>
{
    // the equivalent raw_type
    using raw_type = ::defaults::Point;

    // constructor
    _Point(const std::string& name, const std::string& description,
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
    _Point & operator=(const _Point &other)
    {
        x = other.x;
        y = other.y;
        return *this;
    }

    // copy operator from raw_type
    _Point & operator=(const raw_type &other)
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
} // namespace defaults

#endif // DEFAULTS_POINT_H_
