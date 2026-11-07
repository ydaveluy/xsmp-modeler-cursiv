// -----------------------------------------------------------------------------
// File Name    : Holder.h
// -----------------------------------------------------------------------------
/// @file order/Holder.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef ORDER_HOLDER_H_
#define ORDER_HOLDER_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <order/Value.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <Smp/ViewKind.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace order
{
struct Holder {
    ::order::Value first;
    ::order::Value second;

    static void _Register(::Smp::Publication::ITypeRegistry* registry);

    template<typename _BASE> struct _Field : public _BASE {
        _Field(::Smp::Publication::ITypeRegistry *typeRegistry,
               ::Smp::Uuid typeUuid, ::Smp::String8 name,
               ::Smp::String8 description = "", ::Smp::IObject *parent = nullptr,
               ::Smp::ViewKind view = ::Smp::ViewKind::VK_All)
            : _Field(typeRegistry, typeUuid, name, description, parent, view,
                     ::order::Holder{}) {}
        _Field(::Smp::Publication::ITypeRegistry *typeRegistry, ::Smp::Uuid typeUuid,
               ::Smp::String8 name, ::Smp::String8 description, ::Smp::IObject *parent,
               ::Smp::ViewKind view, const ::order::Holder &value) :
            _BASE(typeRegistry, typeUuid, name, description, parent, view),
            // Field first
            first{
                typeRegistry, // Type Registry
                ::order::Uuid_Value, //Type UUID
                "first", // Name
                "", // Description
                this, // Parent
                view, // View Kind
                value.first // Value
            },
            // Field second
            second{
                typeRegistry, // Type Registry
                ::order::Uuid_Value, //Type UUID
                "second", // Name
                "", // Description
                this, // Parent
                view, // View Kind
                value.second // Value
            }
        {
        }
        _Field(const _Field&) = delete;
        _Field& operator=(const _Field&) = delete;

        // copy operator from ::order::Holder
        _Field & operator=(const ::order::Holder &other) {
            this->first = other.first;
            this->second = other.second;
            return *this;
        }

        // implicit convertion to ::order::Holder
        operator ::order::Holder() const noexcept {
            return {first, second};
        }

        // Fields declaration
        typename _BASE::template Field<::order::Value> first;
        typename _BASE::template Field<::order::Value> second;
    };
};

/// Universally unique identifier of type Holder.
inline constexpr ::Smp::Uuid Uuid_Holder { 0x83000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000004U };
} // namespace order

#endif // ORDER_HOLDER_H_
