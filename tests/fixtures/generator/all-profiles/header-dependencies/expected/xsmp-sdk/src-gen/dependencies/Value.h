// -----------------------------------------------------------------------------
// File Name    : Value.h
// -----------------------------------------------------------------------------
/// @file dependencies/Value.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef DEPENDENCIES_VALUE_H_
#define DEPENDENCIES_VALUE_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <Smp/ViewKind.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace dependencies
{
struct Value {
    ::Smp::Int32 number;

    static void _Register(::Smp::Publication::ITypeRegistry* registry);

    template<typename _BASE> struct _Field : public _BASE {
        _Field(::Smp::Publication::ITypeRegistry *typeRegistry,
               ::Smp::Uuid typeUuid, ::Smp::String8 name,
               ::Smp::String8 description = "", ::Smp::IObject *parent = nullptr,
               ::Smp::ViewKind view = ::Smp::ViewKind::VK_All)
            : _Field(typeRegistry, typeUuid, name, description, parent, view,
                     ::dependencies::Value{}) {}
        _Field(::Smp::Publication::ITypeRegistry *typeRegistry, ::Smp::Uuid typeUuid,
               ::Smp::String8 name, ::Smp::String8 description, ::Smp::IObject *parent,
               ::Smp::ViewKind view, const ::dependencies::Value &value) :
            _BASE(typeRegistry, typeUuid, name, description, parent, view),
            // Field number
            number{
                typeRegistry, // Type Registry
                ::Smp::Uuids::Uuid_Int32, //Type UUID
                "number", // Name
                "", // Description
                this, // Parent
                view, // View Kind
                value.number // Value
            }
        {
        }
        _Field(const _Field&) = delete;
        _Field& operator=(const _Field&) = delete;

        // copy operator from ::dependencies::Value
        _Field & operator=(const ::dependencies::Value &other) {
            this->number = other.number;
            return *this;
        }

        // implicit convertion to ::dependencies::Value
        operator ::dependencies::Value() const noexcept {
            return {number};
        }

        // Fields declaration
        typename _BASE::template Field<::Smp::Int32> number;
    };
};

/// Universally unique identifier of type Value.
inline constexpr ::Smp::Uuid Uuid_Value { 0x82000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000005U };
} // namespace dependencies

#endif // DEPENDENCIES_VALUE_H_
