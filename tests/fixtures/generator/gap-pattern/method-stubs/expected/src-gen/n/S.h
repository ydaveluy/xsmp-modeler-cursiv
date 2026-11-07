// -----------------------------------------------------------------------------
// File Name    : S.h
// -----------------------------------------------------------------------------
/// @file n/S.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef N_S_H_
#define N_S_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <n/Huge.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <Smp/ViewKind.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace n
{
struct S {
    ::n::Huge items;

    static void _Register(::Smp::Publication::ITypeRegistry* registry);

    template<typename _BASE> struct _Field : public _BASE {
        _Field(::Smp::Publication::ITypeRegistry *typeRegistry,
               ::Smp::Uuid typeUuid, ::Smp::String8 name,
               ::Smp::String8 description = "", ::Smp::IObject *parent = nullptr,
               ::Smp::ViewKind view = ::Smp::ViewKind::VK_All)
            : _Field(typeRegistry, typeUuid, name, description, parent, view,
                     ::n::S{}) {}
        _Field(::Smp::Publication::ITypeRegistry *typeRegistry, ::Smp::Uuid typeUuid,
               ::Smp::String8 name, ::Smp::String8 description, ::Smp::IObject *parent,
               ::Smp::ViewKind view, const ::n::S &value) :
            _BASE(typeRegistry, typeUuid, name, description, parent, view),
            // Field items
            items{
                typeRegistry, // Type Registry
                ::n::Uuid_Huge, //Type UUID
                "items", // Name
                "", // Description
                this, // Parent
                view, // View Kind
                value.items // Value
            }
        {
        }
        _Field(const _Field&) = delete;
        _Field& operator=(const _Field&) = delete;

        // copy operator from ::n::S
        _Field & operator=(const ::n::S &other) {
            this->items = other.items;
            return *this;
        }

        // implicit convertion to ::n::S
        operator ::n::S() const noexcept {
            return {items};
        }

        // Fields declaration
        typename _BASE::template Field<::n::Huge> items;
    };
};

/// Universally unique identifier of type S.
inline constexpr ::Smp::Uuid Uuid_S { 0x11111111U, 0x0000U, 0x4000U, 0x8000U, 0x0000000000f3U };
} // namespace n

#endif // N_S_H_
