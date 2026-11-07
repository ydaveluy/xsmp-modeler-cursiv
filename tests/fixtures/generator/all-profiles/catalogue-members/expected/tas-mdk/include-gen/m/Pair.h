// Block notice
// line two
// -----------------------------------------------------------------------------
// File Name    : Pair.h
// -----------------------------------------------------------------------------
/// @file m/Pair.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef M_PAIR_H_
#define M_PAIR_H_

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

namespace m
{
struct Pair
{
    ::Smp::Int32 a{1};
    ::Smp::Int32 b;

    Pair() = default;
    ~Pair() = default;
    Pair(const Pair&) = default;
    Pair(Pair&&) = default;
    Pair(::Smp::Int32 a, ::Smp::Int32 b):
    a(a), b(b) {}
    Pair& operator=(const Pair&) = default;

    static void _Register(::Smp::Publication::ITypeRegistry* registry);
};

/// Universally unique identifier of type Pair.
constexpr ::Smp::Uuid Uuid_Pair { 0x20000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000002U };

template<bool ...Opts>
struct _Pair : public ::TasMdk::Types::StructureType<Opts...>
{
    // the equivalent raw_type
    using raw_type = ::m::Pair;

    // constructor
    _Pair(const std::string& name, const std::string& description,
            Smp::IObject *parent, Smp::ViewKind view,
            const Smp::Publication::ITypeRegistry *type_registry, Smp::Uuid typeUuid,
            const raw_type &default_value = raw_type{}) :
            ::TasMdk::Types::StructureType<Opts...>(name, description, parent, view, type_registry->GetType(typeUuid)),
            /// a initialization
            a{"a", "", this, view, type_registry, ::Smp::Uuids::Uuid_Int32, default_value.a},
            /// b initialization
            b{"b", "", this, view, type_registry, ::Smp::Uuids::Uuid_Int32, default_value.b} {
    }

    // copy operator
    _Pair & operator=(const _Pair &other)
    {
        a = other.a;
        b = other.b;
        return *this;
    }

    // copy operator from raw_type
    _Pair & operator=(const raw_type &other)
    {
        a = other.a;
        b = other.b;
        return *this;
    }

    // convert to raw_type
    operator raw_type() const noexcept
    {
        return {a, b};
    }
    typename ::TasMdk::SimpleField<::Smp::Int32>::in<Opts...>::type a;
    typename ::TasMdk::SimpleField<::Smp::Int32>::in<Opts...>::type b;
};
} // namespace m

#endif // M_PAIR_H_
