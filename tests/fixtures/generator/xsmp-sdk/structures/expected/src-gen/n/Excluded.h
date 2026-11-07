// -----------------------------------------------------------------------------
// File Name    : Excluded.h
// -----------------------------------------------------------------------------
/// @file n/Excluded.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef N_EXCLUDED_H_
#define N_EXCLUDED_H_

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
struct Excluded {
    static ::Smp::Int32 shared;
    ::Smp::String8 label;
    ::Smp::Int32 kept;

    static void _Register(::Smp::Publication::ITypeRegistry* registry);

    template<typename _BASE> struct _Field : public _BASE {
        _Field(::Smp::Publication::ITypeRegistry *typeRegistry,
               ::Smp::Uuid typeUuid, ::Smp::String8 name,
               ::Smp::String8 description = "", ::Smp::IObject *parent = nullptr,
               ::Smp::ViewKind view = ::Smp::ViewKind::VK_All)
            : _Field(typeRegistry, typeUuid, name, description, parent, view,
                     ::n::Excluded{}) {}
        _Field(::Smp::Publication::ITypeRegistry *typeRegistry, ::Smp::Uuid typeUuid,
               ::Smp::String8 name, ::Smp::String8 description, ::Smp::IObject *parent,
               ::Smp::ViewKind view, const ::n::Excluded &value) :
            _BASE(typeRegistry, typeUuid, name, description, parent, view),
            // Field label
            label{
                typeRegistry, // Type Registry
                ::Smp::Uuids::Uuid_String8, //Type UUID
                "label", // Name
                "", // Description
                this, // Parent
                view, // View Kind
                value.label // Value
            },
            // Field kept
            kept{
                typeRegistry, // Type Registry
                ::Smp::Uuids::Uuid_Int32, //Type UUID
                "kept", // Name
                "", // Description
                this, // Parent
                view, // View Kind
                value.kept // Value
            }
        {
        }
        _Field(const _Field&) = delete;
        _Field& operator=(const _Field&) = delete;

        // copy operator from ::n::Excluded
        _Field & operator=(const ::n::Excluded &other) {
            this->label = other.label;
            this->kept = other.kept;
            return *this;
        }

        // implicit convertion to ::n::Excluded
        operator ::n::Excluded() const noexcept {
            return {label, kept};
        }

        // Fields declaration
        typename _BASE::template Field<::Smp::String8> label;
        typename _BASE::template Field<::Smp::Int32> kept;
    };
};

/// Universally unique identifier of type Excluded.
inline constexpr ::Smp::Uuid Uuid_Excluded { 0x11111111U, 0x0000U, 0x4000U, 0x8000U, 0x000000000032U };
} // namespace n

#endif // N_EXCLUDED_H_
