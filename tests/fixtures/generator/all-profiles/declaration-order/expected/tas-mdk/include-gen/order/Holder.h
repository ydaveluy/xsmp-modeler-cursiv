// -----------------------------------------------------------------------------
// File Name    : Holder.h
// -----------------------------------------------------------------------------
/// @file order/Holder.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef ORDER_HOLDER_H_
#define ORDER_HOLDER_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <order/Value.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <Smp/ViewKind.h>
#include <TasMdk/Types/Structure.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace order
{
struct Holder
{
    ::order::Value first;
    ::order::Value second;

    static void _Register(::Smp::Publication::ITypeRegistry* registry);
};

/// Universally unique identifier of type Holder.
constexpr ::Smp::Uuid Uuid_Holder { 0x83000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000004U };

template<bool ...Opts>
struct _Holder : public ::TasMdk::Types::StructureType<Opts...>
{
    // the equivalent raw_type
    using raw_type = ::order::Holder;

    // constructor
    _Holder(const std::string& name, const std::string& description,
            Smp::IObject *parent, Smp::ViewKind view,
            const Smp::Publication::ITypeRegistry *type_registry, Smp::Uuid typeUuid,
            const raw_type &default_value = raw_type{}) :
            ::TasMdk::Types::StructureType<Opts...>(name, description, parent, view, type_registry->GetType(typeUuid)),
            /// first initialization
            first{"first", "", this, view, type_registry, ::order::Uuid_Value, default_value.first},
            /// second initialization
            second{"second", "", this, view, type_registry, ::order::Uuid_Value, default_value.second} {
    }

    // copy operator
    _Holder & operator=(const _Holder &other)
    {
        first = other.first;
        second = other.second;
        return *this;
    }

    // copy operator from raw_type
    _Holder & operator=(const raw_type &other)
    {
        first = other.first;
        second = other.second;
        return *this;
    }

    // convert to raw_type
    operator raw_type() const noexcept
    {
        return {first, second};
    }
    typename ::TasMdk::StructureField<::order::_Value>::in<Opts...>::type first;
    typename ::TasMdk::StructureField<::order::_Value>::in<Opts...>::type second;
};
} // namespace order

#endif // ORDER_HOLDER_H_
