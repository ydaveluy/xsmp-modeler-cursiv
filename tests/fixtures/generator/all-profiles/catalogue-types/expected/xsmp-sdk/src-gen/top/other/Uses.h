// Copyright line one
// Copyright line two
// -----------------------------------------------------------------------------
// File Name    : Uses.h
// -----------------------------------------------------------------------------
/// @file top/other/Uses.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef TOP_OTHER_USES_H_
#define TOP_OTHER_USES_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <Smp/ViewKind.h>
#include <top/inner/Colour.h>
#include <top/inner/Payload.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace top::other
{
struct Uses {
    static constexpr ::Smp::Int32 V{::top::inner::Payload::Answer + 1};
    ::top::inner::Payload p;
    ::top::inner::Colour c{::top::inner::Colour::Red};

    static void _Register(::Smp::Publication::ITypeRegistry* registry);

    template<typename _BASE> struct _Field : public _BASE {
        _Field(::Smp::Publication::ITypeRegistry *typeRegistry,
               ::Smp::Uuid typeUuid, ::Smp::String8 name,
               ::Smp::String8 description = "", ::Smp::IObject *parent = nullptr,
               ::Smp::ViewKind view = ::Smp::ViewKind::VK_All)
            : _Field(typeRegistry, typeUuid, name, description, parent, view,
                     ::top::other::Uses{}) {}
        _Field(::Smp::Publication::ITypeRegistry *typeRegistry, ::Smp::Uuid typeUuid,
               ::Smp::String8 name, ::Smp::String8 description, ::Smp::IObject *parent,
               ::Smp::ViewKind view, const ::top::other::Uses &value) :
            _BASE(typeRegistry, typeUuid, name, description, parent, view),
            // Field p
            p{
                typeRegistry, // Type Registry
                ::top::inner::Uuid_Payload, //Type UUID
                "p", // Name
                "", // Description
                this, // Parent
                view, // View Kind
                value.p // Value
            },
            // Field c
            c{
                typeRegistry, // Type Registry
                ::top::inner::Uuid_Colour, //Type UUID
                "c", // Name
                "", // Description
                this, // Parent
                view, // View Kind
                value.c // Value
            }
        {
        }
        _Field(const _Field&) = delete;
        _Field& operator=(const _Field&) = delete;

        // copy operator from ::top::other::Uses
        _Field & operator=(const ::top::other::Uses &other) {
            this->p = other.p;
            this->c = other.c;
            return *this;
        }

        // implicit convertion to ::top::other::Uses
        operator ::top::other::Uses() const noexcept {
            return {p, c};
        }

        // Fields declaration
        typename _BASE::template Field<::top::inner::Payload> p;
        typename _BASE::template Field<::top::inner::Colour> c;
    };
};

/// Universally unique identifier of type Uses.
inline constexpr ::Smp::Uuid Uuid_Uses { 0x10000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000020U };
} // namespace top::other

#endif // TOP_OTHER_USES_H_
