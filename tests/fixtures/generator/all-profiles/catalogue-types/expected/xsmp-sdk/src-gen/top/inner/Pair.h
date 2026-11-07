// Copyright line one
// Copyright line two
// -----------------------------------------------------------------------------
// File Name    : Pair.h
// -----------------------------------------------------------------------------
/// @file top/inner/Pair.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef TOP_INNER_PAIR_H_
#define TOP_INNER_PAIR_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <Smp/ViewKind.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

/// Inner namespace doc.
///  Second line.
namespace top::inner
{
/// A structure with designated initializers.
struct Pair {
    ::Smp::Int32 x;
    ::Smp::Int32 y;

    static void _Register(::Smp::Publication::ITypeRegistry* registry);

    template<typename _BASE> struct _Field : public _BASE {
        _Field(::Smp::Publication::ITypeRegistry *typeRegistry,
               ::Smp::Uuid typeUuid, ::Smp::String8 name,
               ::Smp::String8 description = "", ::Smp::IObject *parent = nullptr,
               ::Smp::ViewKind view = ::Smp::ViewKind::VK_All)
            : _Field(typeRegistry, typeUuid, name, description, parent, view,
                     ::top::inner::Pair{}) {}
        _Field(::Smp::Publication::ITypeRegistry *typeRegistry, ::Smp::Uuid typeUuid,
               ::Smp::String8 name, ::Smp::String8 description, ::Smp::IObject *parent,
               ::Smp::ViewKind view, const ::top::inner::Pair &value) :
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
            },
            // Field y
            y{
                typeRegistry, // Type Registry
                ::Smp::Uuids::Uuid_Int32, //Type UUID
                "y", // Name
                "", // Description
                this, // Parent
                view, // View Kind
                value.y // Value
            }
        {
        }
        _Field(const _Field&) = delete;
        _Field& operator=(const _Field&) = delete;

        // copy operator from ::top::inner::Pair
        _Field & operator=(const ::top::inner::Pair &other) {
            this->x = other.x;
            this->y = other.y;
            return *this;
        }

        // implicit convertion to ::top::inner::Pair
        operator ::top::inner::Pair() const noexcept {
            return {x, y};
        }

        // Fields declaration
        typename _BASE::template Field<::Smp::Int32> x;
        typename _BASE::template Field<::Smp::Int32> y;
    };
};

/// Universally unique identifier of type Pair.
inline constexpr ::Smp::Uuid Uuid_Pair { 0x10000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000027U };
} // namespace top::inner

#endif // TOP_INNER_PAIR_H_
