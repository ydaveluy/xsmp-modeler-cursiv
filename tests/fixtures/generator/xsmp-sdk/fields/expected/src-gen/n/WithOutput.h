// -----------------------------------------------------------------------------
// File Name    : WithOutput.h
// -----------------------------------------------------------------------------
/// @file n/WithOutput.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef N_WITHOUTPUT_H_
#define N_WITHOUTPUT_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <Smp/ViewKind.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace n
{
struct WithOutput {
    ::Smp::Int32 x;

    static void _Register(::Smp::Publication::ITypeRegistry* registry);

    template<typename _BASE> struct _Field : public _BASE {
        _Field(::Smp::Publication::ITypeRegistry *typeRegistry,
               ::Smp::Uuid typeUuid, ::Smp::String8 name,
               ::Smp::String8 description = "", ::Smp::IObject *parent = nullptr,
               ::Smp::ViewKind view = ::Smp::ViewKind::VK_All)
            : _Field(typeRegistry, typeUuid, name, description, parent, view,
                     ::n::WithOutput{}) {}
        _Field(::Smp::Publication::ITypeRegistry *typeRegistry, ::Smp::Uuid typeUuid,
               ::Smp::String8 name, ::Smp::String8 description, ::Smp::IObject *parent,
               ::Smp::ViewKind view, const ::n::WithOutput &value) :
            _BASE(typeRegistry, typeUuid, name, description, parent, view),
            // Field x
            x{
                typeRegistry, // Type Registry
                ::Smp::Uuids::Uuid_Int32, //Type UUID
                "x", // Name
                "", // Description
                this, // Parent
                view, // View Kind
                value.x // Value
            }
        {
        }
        _Field(const _Field&) = delete;
        _Field& operator=(const _Field&) = delete;

        // copy operator from ::n::WithOutput
        _Field & operator=(const ::n::WithOutput &other) {
            this->x = other.x;
            return *this;
        }

        // implicit convertion to ::n::WithOutput
        operator ::n::WithOutput() const noexcept {
            return {x};
        }

        // Fields declaration
        typename _BASE::template Field<::Smp::Int32>::output x;
    };
};

/// Universally unique identifier of type WithOutput.
inline constexpr ::Smp::Uuid Uuid_WithOutput { 0x11111111U, 0x0000U, 0x4000U, 0x8000U, 0x000000000082U };
} // namespace n

#endif // N_WITHOUTPUT_H_
