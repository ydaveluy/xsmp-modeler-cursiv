// Copyright line one
// Copyright line two
// -----------------------------------------------------------------------------
// File Name    : Earlier.h
// -----------------------------------------------------------------------------
/// @file top/other/Earlier.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef TOP_OTHER_EARLIER_H_
#define TOP_OTHER_EARLIER_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <Smp/Publication/ITypeRegistry.h>
#include <Smp/ViewKind.h>
#include <top/other/Later2.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace top::other
{
struct Earlier {
    ::top::other::Later2 x;

    static void _Register(::Smp::Publication::ITypeRegistry* registry);

    template<typename _BASE> struct _Field : public _BASE {
        _Field(::Smp::Publication::ITypeRegistry *typeRegistry,
               ::Smp::Uuid typeUuid, ::Smp::String8 name,
               ::Smp::String8 description = "", ::Smp::IObject *parent = nullptr,
               ::Smp::ViewKind view = ::Smp::ViewKind::VK_All)
            : _Field(typeRegistry, typeUuid, name, description, parent, view,
                     ::top::other::Earlier{}) {}
        _Field(::Smp::Publication::ITypeRegistry *typeRegistry, ::Smp::Uuid typeUuid,
               ::Smp::String8 name, ::Smp::String8 description, ::Smp::IObject *parent,
               ::Smp::ViewKind view, const ::top::other::Earlier &value) :
            _BASE(typeRegistry, typeUuid, name, description, parent, view),
            // Field x
            x{
                typeRegistry, // Type Registry
                ::top::other::Uuid_Later2, //Type UUID
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

        // copy operator from ::top::other::Earlier
        _Field & operator=(const ::top::other::Earlier &other) {
            this->x = other.x;
            return *this;
        }

        // implicit convertion to ::top::other::Earlier
        operator ::top::other::Earlier() const noexcept {
            return {x};
        }

        // Fields declaration
        typename _BASE::template Field<::top::other::Later2> x;
    };
};

/// Universally unique identifier of type Earlier.
inline constexpr ::Smp::Uuid Uuid_Earlier { 0x10000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000022U };
} // namespace top::other

#endif // TOP_OTHER_EARLIER_H_
