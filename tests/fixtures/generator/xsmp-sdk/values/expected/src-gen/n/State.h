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

#include <cmath>
#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <Smp/ViewKind.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace n
{
struct State {
    static constexpr ::Smp::Float64 Scale{-1.0};
    ::Smp::Float64 ratio{Scale + 1.0};
    ::Smp::Duration cycle{1500000000L};
    ::Smp::DateTime epoch{719701664101010101L};

    static void _Register(::Smp::Publication::ITypeRegistry* registry);

    template<typename _BASE> struct _Field : public _BASE {
        _Field(::Smp::Publication::ITypeRegistry *typeRegistry,
               ::Smp::Uuid typeUuid, ::Smp::String8 name,
               ::Smp::String8 description = "", ::Smp::IObject *parent = nullptr,
               ::Smp::ViewKind view = ::Smp::ViewKind::VK_All)
            : _Field(typeRegistry, typeUuid, name, description, parent, view,
                     ::n::State{}) {}
        _Field(::Smp::Publication::ITypeRegistry *typeRegistry, ::Smp::Uuid typeUuid,
               ::Smp::String8 name, ::Smp::String8 description, ::Smp::IObject *parent,
               ::Smp::ViewKind view, const ::n::State &value) :
            _BASE(typeRegistry, typeUuid, name, description, parent, view),
            // Field ratio
            ratio{
                typeRegistry, // Type Registry
                ::Smp::Uuids::Uuid_Float64, //Type UUID
                "ratio", // Name
                "", // Description
                this, // Parent
                view, // View Kind
                value.ratio // Value
            },
            // Field cycle
            cycle{
                typeRegistry, // Type Registry
                ::Smp::Uuids::Uuid_Duration, //Type UUID
                "cycle", // Name
                "", // Description
                this, // Parent
                view, // View Kind
                value.cycle // Value
            },
            // Field epoch
            epoch{
                typeRegistry, // Type Registry
                ::Smp::Uuids::Uuid_DateTime, //Type UUID
                "epoch", // Name
                "", // Description
                this, // Parent
                view, // View Kind
                value.epoch // Value
            }
        {
        }
        _Field(const _Field&) = delete;
        _Field& operator=(const _Field&) = delete;

        // copy operator from ::n::State
        _Field & operator=(const ::n::State &other) {
            this->ratio = other.ratio;
            this->cycle = other.cycle;
            this->epoch = other.epoch;
            return *this;
        }

        // implicit convertion to ::n::State
        operator ::n::State() const noexcept {
            return {ratio, cycle, epoch};
        }

        // Fields declaration
        typename _BASE::template Field<::Smp::Float64> ratio;
        typename _BASE::template Field<::Smp::Duration> cycle;
        typename _BASE::template Field<::Smp::DateTime> epoch;
    };
};

/// Universally unique identifier of type State.
inline constexpr ::Smp::Uuid Uuid_State { 0x11111111U, 0x0000U, 0x4000U, 0x8000U, 0x000000000091U };
} // namespace n

#endif // N_STATE_H_
