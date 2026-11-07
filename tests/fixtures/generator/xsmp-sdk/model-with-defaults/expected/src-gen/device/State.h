// -----------------------------------------------------------------------------
// File Name    : State.h
// -----------------------------------------------------------------------------
/// @file device/State.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef DEVICE_STATE_H_
#define DEVICE_STATE_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <device/Mode.h>
#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <Smp/ViewKind.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace device
{
struct State {
    ::Smp::Float64 position{3.141592653589793};
    ::device::Mode mode{::device::Mode::Off};

    static void _Register(::Smp::Publication::ITypeRegistry* registry);

    template<typename _BASE> struct _Field : public _BASE {
        _Field(::Smp::Publication::ITypeRegistry *typeRegistry,
               ::Smp::Uuid typeUuid, ::Smp::String8 name,
               ::Smp::String8 description = "", ::Smp::IObject *parent = nullptr,
               ::Smp::ViewKind view = ::Smp::ViewKind::VK_All)
            : _Field(typeRegistry, typeUuid, name, description, parent, view,
                     ::device::State{}) {}
        _Field(::Smp::Publication::ITypeRegistry *typeRegistry, ::Smp::Uuid typeUuid,
               ::Smp::String8 name, ::Smp::String8 description, ::Smp::IObject *parent,
               ::Smp::ViewKind view, const ::device::State &value) :
            _BASE(typeRegistry, typeUuid, name, description, parent, view),
            // Field position
            position{
                typeRegistry, // Type Registry
                ::Smp::Uuids::Uuid_Float64, //Type UUID
                "position", // Name
                "", // Description
                this, // Parent
                view, // View Kind
                value.position // Value
            },
            // Field mode
            mode{
                typeRegistry, // Type Registry
                ::device::Uuid_Mode, //Type UUID
                "mode", // Name
                "", // Description
                this, // Parent
                view, // View Kind
                value.mode // Value
            }
        {
        }
        _Field(const _Field&) = delete;
        _Field& operator=(const _Field&) = delete;

        // copy operator from ::device::State
        _Field & operator=(const ::device::State &other) {
            this->position = other.position;
            this->mode = other.mode;
            return *this;
        }

        // implicit convertion to ::device::State
        operator ::device::State() const noexcept {
            return {position, mode};
        }

        // Fields declaration
        typename _BASE::template Field<::Smp::Float64> position;
        typename _BASE::template Field<::device::Mode> mode;
    };
};

/// Universally unique identifier of type State.
inline constexpr ::Smp::Uuid Uuid_State { 0x22222222U, 0x2222U, 0x4222U, 0x8222U, 0x222222222222U };
} // namespace device

#endif // DEVICE_STATE_H_
