// Block notice
// line two
// -----------------------------------------------------------------------------
// File Name    : Pair.h
// -----------------------------------------------------------------------------
/// @file m/Pair.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef M_PAIR_H_
#define M_PAIR_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <Smp/ViewKind.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace m
{
struct Pair {
    ::Smp::Int32 a{1};
    ::Smp::Int32 b;

    static void _Register(::Smp::Publication::ITypeRegistry* registry);

    template<typename _BASE> struct _Field : public _BASE {
        _Field(::Smp::Publication::ITypeRegistry *typeRegistry,
               ::Smp::Uuid typeUuid, ::Smp::String8 name,
               ::Smp::String8 description = "", ::Smp::IObject *parent = nullptr,
               ::Smp::ViewKind view = ::Smp::ViewKind::VK_All)
            : _Field(typeRegistry, typeUuid, name, description, parent, view,
                     ::m::Pair{}) {}
        _Field(::Smp::Publication::ITypeRegistry *typeRegistry, ::Smp::Uuid typeUuid,
               ::Smp::String8 name, ::Smp::String8 description, ::Smp::IObject *parent,
               ::Smp::ViewKind view, const ::m::Pair &value) :
            _BASE(typeRegistry, typeUuid, name, description, parent, view),
            // Field a
            a{
                typeRegistry, // Type Registry
                ::Smp::Uuids::Uuid_Int32, //Type UUID
                "a", // Name
                "", // Description
                this, // Parent
                view, // View Kind
                value.a // Value
            },
            // Field b
            b{
                typeRegistry, // Type Registry
                ::Smp::Uuids::Uuid_Int32, //Type UUID
                "b", // Name
                "", // Description
                this, // Parent
                view, // View Kind
                value.b // Value
            }
        {
        }
        _Field(const _Field&) = delete;
        _Field& operator=(const _Field&) = delete;

        // copy operator from ::m::Pair
        _Field & operator=(const ::m::Pair &other) {
            this->a = other.a;
            this->b = other.b;
            return *this;
        }

        // implicit convertion to ::m::Pair
        operator ::m::Pair() const noexcept {
            return {a, b};
        }

        // Fields declaration
        typename _BASE::template Field<::Smp::Int32> a;
        typename _BASE::template Field<::Smp::Int32> b;
    };
};

/// Universally unique identifier of type Pair.
inline constexpr ::Smp::Uuid Uuid_Pair { 0x20000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000002U };
} // namespace m

#endif // M_PAIR_H_
