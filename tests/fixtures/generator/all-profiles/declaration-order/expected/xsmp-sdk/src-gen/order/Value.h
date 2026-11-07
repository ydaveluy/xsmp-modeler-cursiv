// -----------------------------------------------------------------------------
// File Name    : Value.h
// -----------------------------------------------------------------------------
/// @file order/Value.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef ORDER_VALUE_H_
#define ORDER_VALUE_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <Smp/ViewKind.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace order
{
struct Value {
    ::Smp::Int32 value;

    static void _Register(::Smp::Publication::ITypeRegistry* registry);

    template<typename _BASE> struct _Field : public _BASE {
        _Field(::Smp::Publication::ITypeRegistry *typeRegistry,
               ::Smp::Uuid typeUuid, ::Smp::String8 name,
               ::Smp::String8 description = "", ::Smp::IObject *parent = nullptr,
               ::Smp::ViewKind view = ::Smp::ViewKind::VK_All)
            : _Field(typeRegistry, typeUuid, name, description, parent, view,
                     ::order::Value{}) {}
        _Field(::Smp::Publication::ITypeRegistry *typeRegistry, ::Smp::Uuid typeUuid,
               ::Smp::String8 name, ::Smp::String8 description, ::Smp::IObject *parent,
               ::Smp::ViewKind view, const ::order::Value &value) :
            _BASE(typeRegistry, typeUuid, name, description, parent, view),
            // Field value
            value{
                typeRegistry, // Type Registry
                ::Smp::Uuids::Uuid_Int32, //Type UUID
                "value", // Name
                "", // Description
                this, // Parent
                view, // View Kind
                value.value // Value
            }
        {
        }
        _Field(const _Field&) = delete;
        _Field& operator=(const _Field&) = delete;

        // copy operator from ::order::Value
        _Field & operator=(const ::order::Value &other) {
            this->value = other.value;
            return *this;
        }

        // implicit convertion to ::order::Value
        operator ::order::Value() const noexcept {
            return {value};
        }

        // Fields declaration
        typename _BASE::template Field<::Smp::Int32> value;
    };
};

/// Universally unique identifier of type Value.
inline constexpr ::Smp::Uuid Uuid_Value { 0x83000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000006U };
} // namespace order

#endif // ORDER_VALUE_H_
