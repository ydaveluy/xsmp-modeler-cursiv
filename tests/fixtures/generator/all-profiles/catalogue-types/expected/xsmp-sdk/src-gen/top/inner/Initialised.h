// Copyright line one
// Copyright line two
// -----------------------------------------------------------------------------
// File Name    : Initialised.h
// -----------------------------------------------------------------------------
/// @file top/inner/Initialised.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef TOP_INNER_INITIALISED_H_
#define TOP_INNER_INITIALISED_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <Smp/Publication/ITypeRegistry.h>
#include <Smp/ViewKind.h>
#include <top/inner/Pair.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

/// Inner namespace doc.
///  Second line.
namespace top::inner
{
struct Initialised {
    ::top::inner::Pair byName{ /* .x = */1, /* .y = */2};
    ::top::inner::Pair byPosition{ 3, 4};

    static void _Register(::Smp::Publication::ITypeRegistry* registry);

    template<typename _BASE> struct _Field : public _BASE {
        _Field(::Smp::Publication::ITypeRegistry *typeRegistry,
               ::Smp::Uuid typeUuid, ::Smp::String8 name,
               ::Smp::String8 description = "", ::Smp::IObject *parent = nullptr,
               ::Smp::ViewKind view = ::Smp::ViewKind::VK_All)
            : _Field(typeRegistry, typeUuid, name, description, parent, view,
                     ::top::inner::Initialised{}) {}
        _Field(::Smp::Publication::ITypeRegistry *typeRegistry, ::Smp::Uuid typeUuid,
               ::Smp::String8 name, ::Smp::String8 description, ::Smp::IObject *parent,
               ::Smp::ViewKind view, const ::top::inner::Initialised &value) :
            _BASE(typeRegistry, typeUuid, name, description, parent, view),
            // Field byName
            byName{
                typeRegistry, // Type Registry
                ::top::inner::Uuid_Pair, //Type UUID
                "byName", // Name
                "", // Description
                this, // Parent
                view, // View Kind
                value.byName // Value
            },
            // Field byPosition
            byPosition{
                typeRegistry, // Type Registry
                ::top::inner::Uuid_Pair, //Type UUID
                "byPosition", // Name
                "", // Description
                this, // Parent
                view, // View Kind
                value.byPosition // Value
            }
        {
        }
        _Field(const _Field&) = delete;
        _Field& operator=(const _Field&) = delete;

        // copy operator from ::top::inner::Initialised
        _Field & operator=(const ::top::inner::Initialised &other) {
            this->byName = other.byName;
            this->byPosition = other.byPosition;
            return *this;
        }

        // implicit convertion to ::top::inner::Initialised
        operator ::top::inner::Initialised() const noexcept {
            return {byName, byPosition};
        }

        // Fields declaration
        typename _BASE::template Field<::top::inner::Pair> byName;
        typename _BASE::template Field<::top::inner::Pair> byPosition;
    };
};

/// Universally unique identifier of type Initialised.
inline constexpr ::Smp::Uuid Uuid_Initialised { 0x10000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000028U };
} // namespace top::inner

#endif // TOP_INNER_INITIALISED_H_
