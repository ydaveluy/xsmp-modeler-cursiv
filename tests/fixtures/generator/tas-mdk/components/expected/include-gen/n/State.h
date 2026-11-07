// -----------------------------------------------------------------------------
// File Name    : State.h
// -----------------------------------------------------------------------------
/// @file n/State.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef N_STATE_H_
#define N_STATE_H_

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

namespace n
{
struct State
{
    ::Smp::Int32 value;

    static void _Register(::Smp::Publication::ITypeRegistry* registry);
};

/// Universally unique identifier of type State.
constexpr ::Smp::Uuid Uuid_State { 0x11111111U, 0x0000U, 0x4000U, 0x8000U, 0x000000000143U };

template<bool ...Opts>
struct _State : public ::TasMdk::Types::StructureType<Opts...>
{
    // the equivalent raw_type
    using raw_type = ::n::State;

    // constructor
    _State(const std::string& name, const std::string& description,
            Smp::IObject *parent, Smp::ViewKind view,
            const Smp::Publication::ITypeRegistry *type_registry, Smp::Uuid typeUuid,
            const raw_type &default_value = raw_type{}) :
            ::TasMdk::Types::StructureType<Opts...>(name, description, parent, view, type_registry->GetType(typeUuid)),
            /// value initialization
            value{"value", "", this, view, type_registry, ::Smp::Uuids::Uuid_Int32, default_value.value} {
    }

    // copy operator
    _State & operator=(const _State &other)
    {
        value = other.value;
        return *this;
    }

    // copy operator from raw_type
    _State & operator=(const raw_type &other)
    {
        value = other.value;
        return *this;
    }

    // convert to raw_type
    operator raw_type() const noexcept
    {
        return {value};
    }
    typename ::TasMdk::SimpleField<::Smp::Int32>::in<Opts...>::type value;
};
} // namespace n

#endif // N_STATE_H_
