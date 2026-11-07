// -----------------------------------------------------------------------------
// File Name    : Value.h
// -----------------------------------------------------------------------------
/// @file dependencies/Value.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef DEPENDENCIES_VALUE_H_
#define DEPENDENCIES_VALUE_H_

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

namespace dependencies
{
struct Value
{
    ::Smp::Int32 number;

    static void _Register(::Smp::Publication::ITypeRegistry* registry);
};

/// Universally unique identifier of type Value.
constexpr ::Smp::Uuid Uuid_Value { 0x82000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000005U };

template<bool ...Opts>
struct _Value : public ::TasMdk::Types::StructureType<Opts...>
{
    // the equivalent raw_type
    using raw_type = ::dependencies::Value;

    // constructor
    _Value(const std::string& name, const std::string& description,
            Smp::IObject *parent, Smp::ViewKind view,
            const Smp::Publication::ITypeRegistry *type_registry, Smp::Uuid typeUuid,
            const raw_type &default_value = raw_type{}) :
            ::TasMdk::Types::StructureType<Opts...>(name, description, parent, view, type_registry->GetType(typeUuid)),
            /// number initialization
            number{"number", "", this, view, type_registry, ::Smp::Uuids::Uuid_Int32, default_value.number} {
    }

    // copy operator
    _Value & operator=(const _Value &other)
    {
        number = other.number;
        return *this;
    }

    // copy operator from raw_type
    _Value & operator=(const raw_type &other)
    {
        number = other.number;
        return *this;
    }

    // convert to raw_type
    operator raw_type() const noexcept
    {
        return {number};
    }
    typename ::TasMdk::SimpleField<::Smp::Int32>::in<Opts...>::type number;
};
} // namespace dependencies

#endif // DEPENDENCIES_VALUE_H_
