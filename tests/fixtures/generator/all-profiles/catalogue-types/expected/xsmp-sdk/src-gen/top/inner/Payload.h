// Copyright line one
// Copyright line two
// -----------------------------------------------------------------------------
// File Name    : Payload.h
// -----------------------------------------------------------------------------
/// @file top/inner/Payload.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef TOP_INNER_PAYLOAD_H_
#define TOP_INNER_PAYLOAD_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <Smp/ViewKind.h>
#include <top/inner/Colour.h>
#include <top/inner/Ints.h>
#include <top/inner/Label.h>
#include <top/inner/Matrix.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

/// Inner namespace doc.
///  Second line.
namespace top::inner
{
/// A payload.
struct Payload {
    static constexpr ::Smp::Bool First{true};
    static constexpr ::Smp::Int32 Answer{42};
    static const ::top::inner::Label Greeting;
    static constexpr ::Smp::Bool Second{First};
    /// A flag.
    ::Smp::Bool enabled{false};
    ::Smp::Int32 count{3};
    ::Smp::Float64 ratio{1.5};
    ::top::inner::Colour colour{::top::inner::Colour::Green};
    ::top::inner::Ints values{{ 1, 2, 3}};
    ::top::inner::Matrix grid{{ {{ 1, 2, 3}}, {{ 4, 5, 6}}}};
    ::top::inner::Label name{"bob"};
    ::Smp::String8 lost;
    static ::Smp::Int32 shared;
    mutable ::Smp::Int32 changing{1};
    ::Smp::Duration span{1000000000L};
    ::Smp::DateTime when{757339200000000000L};
    ::Smp::Int8 letter{1};

    static void _Register(::Smp::Publication::ITypeRegistry* registry);

    template<typename _BASE> struct _Field : public _BASE {
        _Field(::Smp::Publication::ITypeRegistry *typeRegistry,
               ::Smp::Uuid typeUuid, ::Smp::String8 name,
               ::Smp::String8 description = "", ::Smp::IObject *parent = nullptr,
               ::Smp::ViewKind view = ::Smp::ViewKind::VK_All)
            : _Field(typeRegistry, typeUuid, name, description, parent, view,
                     ::top::inner::Payload{}) {}
        _Field(::Smp::Publication::ITypeRegistry *typeRegistry, ::Smp::Uuid typeUuid,
               ::Smp::String8 name, ::Smp::String8 description, ::Smp::IObject *parent,
               ::Smp::ViewKind view, const ::top::inner::Payload &value) :
            _BASE(typeRegistry, typeUuid, name, description, parent, view),
            // Field enabled
            enabled{
                typeRegistry, // Type Registry
                ::Smp::Uuids::Uuid_Bool, //Type UUID
                "enabled", // Name
                "A flag.", // Description
                this, // Parent
                view, // View Kind
                value.enabled // Value
            },
            // Field count
            count{
                typeRegistry, // Type Registry
                ::Smp::Uuids::Uuid_Int32, //Type UUID
                "count", // Name
                "", // Description
                this, // Parent
                view, // View Kind
                value.count // Value
            },
            // Field ratio
            ratio{
                typeRegistry, // Type Registry
                ::Smp::Uuids::Uuid_Float64, //Type UUID
                "ratio", // Name
                "", // Description
                this, // Parent
                ::Smp::ViewKind::VK_Debug, // View Kind
                value.ratio // Value
            },
            // Field colour
            colour{
                typeRegistry, // Type Registry
                ::top::inner::Uuid_Colour, //Type UUID
                "colour", // Name
                "", // Description
                this, // Parent
                view, // View Kind
                value.colour // Value
            },
            // Field values
            values{
                typeRegistry, // Type Registry
                ::top::inner::Uuid_Ints, //Type UUID
                "values", // Name
                "", // Description
                this, // Parent
                view, // View Kind
                value.values // Value
            },
            // Field grid
            grid{
                typeRegistry, // Type Registry
                ::top::inner::Uuid_Matrix, //Type UUID
                "grid", // Name
                "", // Description
                this, // Parent
                view, // View Kind
                value.grid // Value
            },
            // Field name
            name{
                typeRegistry, // Type Registry
                ::top::inner::Uuid_Label, //Type UUID
                "name", // Name
                "", // Description
                this, // Parent
                view, // View Kind
                value.name // Value
            },
            // Field lost
            lost{
                typeRegistry, // Type Registry
                ::Smp::Uuids::Uuid_String8, //Type UUID
                "lost", // Name
                "", // Description
                this, // Parent
                view, // View Kind
                value.lost // Value
            },
            // Field changing
            changing{
                typeRegistry, // Type Registry
                ::Smp::Uuids::Uuid_Int32, //Type UUID
                "changing", // Name
                "", // Description
                this, // Parent
                view, // View Kind
                value.changing // Value
            },
            // Field span
            span{
                typeRegistry, // Type Registry
                ::Smp::Uuids::Uuid_Duration, //Type UUID
                "span", // Name
                "", // Description
                this, // Parent
                view, // View Kind
                value.span // Value
            },
            // Field when
            when{
                typeRegistry, // Type Registry
                ::Smp::Uuids::Uuid_DateTime, //Type UUID
                "when", // Name
                "", // Description
                this, // Parent
                view, // View Kind
                value.when // Value
            },
            // Field letter
            letter{
                typeRegistry, // Type Registry
                ::Smp::Uuids::Uuid_Int8, //Type UUID
                "letter", // Name
                "", // Description
                this, // Parent
                view, // View Kind
                value.letter // Value
            }
        {
        }
        _Field(const _Field&) = delete;
        _Field& operator=(const _Field&) = delete;

        // copy operator from ::top::inner::Payload
        _Field & operator=(const ::top::inner::Payload &other) {
            this->enabled = other.enabled;
            this->count = other.count;
            this->ratio = other.ratio;
            this->colour = other.colour;
            this->values = other.values;
            this->grid = other.grid;
            this->name = other.name;
            this->lost = other.lost;
            this->changing = other.changing;
            this->span = other.span;
            this->when = other.when;
            this->letter = other.letter;
            return *this;
        }

        // implicit convertion to ::top::inner::Payload
        operator ::top::inner::Payload() const noexcept {
            return {enabled, count, ratio, colour, values, grid, name, lost, changing, span, when, letter};
        }

        // Fields declaration
        /// A flag.
        typename _BASE::template Field<::Smp::Bool> enabled;
        typename _BASE::template Field<::Smp::Int32>::transient count;
        typename _BASE::template Field<::Smp::Float64>::input::output ratio;
        typename _BASE::template Field<::top::inner::Colour> colour;
        typename _BASE::template Field<::top::inner::Ints> values;
        typename _BASE::template Field<::top::inner::Matrix> grid;
        typename _BASE::template Field<::top::inner::Label> name;
        typename _BASE::template Field<::Smp::String8> lost;
        mutable typename _BASE::template Field<::Smp::Int32> changing;
        typename _BASE::template Field<::Smp::Duration> span;
        typename _BASE::template Field<::Smp::DateTime> when;
        typename _BASE::template Field<::Smp::Int8> letter;
    };
};

/// Universally unique identifier of type Payload.
inline constexpr ::Smp::Uuid Uuid_Payload { 0x10000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000014U };
} // namespace top::inner

#endif // TOP_INNER_PAYLOAD_H_
