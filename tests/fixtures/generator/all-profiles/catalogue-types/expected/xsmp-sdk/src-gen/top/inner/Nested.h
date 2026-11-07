// Copyright line one
// Copyright line two
// -----------------------------------------------------------------------------
// File Name    : Nested.h
// -----------------------------------------------------------------------------
/// @file top/inner/Nested.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef TOP_INNER_NESTED_H_
#define TOP_INNER_NESTED_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <Smp/ViewKind.h>
#include <top/inner/Payload.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

/// Inner namespace doc.
///  Second line.
namespace top::inner
{
struct Nested {
    ::top::inner::Payload inner;
    ::Smp::Bool ok;

    static void _Register(::Smp::Publication::ITypeRegistry* registry);

    template<typename _BASE> struct _Field : public _BASE {
        _Field(::Smp::Publication::ITypeRegistry *typeRegistry,
               ::Smp::Uuid typeUuid, ::Smp::String8 name,
               ::Smp::String8 description = "", ::Smp::IObject *parent = nullptr,
               ::Smp::ViewKind view = ::Smp::ViewKind::VK_All)
            : _Field(typeRegistry, typeUuid, name, description, parent, view,
                     ::top::inner::Nested{}) {}
        _Field(::Smp::Publication::ITypeRegistry *typeRegistry, ::Smp::Uuid typeUuid,
               ::Smp::String8 name, ::Smp::String8 description, ::Smp::IObject *parent,
               ::Smp::ViewKind view, const ::top::inner::Nested &value) :
            _BASE(typeRegistry, typeUuid, name, description, parent, view),
            // Field inner
            inner{
                typeRegistry, // Type Registry
                ::top::inner::Uuid_Payload, //Type UUID
                "inner", // Name
                "", // Description
                this, // Parent
                view, // View Kind
                value.inner // Value
            },
            // Field ok
            ok{
                typeRegistry, // Type Registry
                ::Smp::Uuids::Uuid_Bool, //Type UUID
                "ok", // Name
                "", // Description
                this, // Parent
                view, // View Kind
                value.ok // Value
            }
        {
        }
        _Field(const _Field&) = delete;
        _Field& operator=(const _Field&) = delete;

        // copy operator from ::top::inner::Nested
        _Field & operator=(const ::top::inner::Nested &other) {
            this->inner = other.inner;
            this->ok = other.ok;
            return *this;
        }

        // implicit convertion to ::top::inner::Nested
        operator ::top::inner::Nested() const noexcept {
            return {inner, ok};
        }

        // Fields declaration
        typename _BASE::template Field<::top::inner::Payload> inner;
        typename _BASE::template Field<::Smp::Bool> ok;
    };
};

/// Universally unique identifier of type Nested.
inline constexpr ::Smp::Uuid Uuid_Nested { 0x10000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000015U };
} // namespace top::inner

#endif // TOP_INNER_NESTED_H_
