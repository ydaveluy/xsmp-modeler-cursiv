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
#include <TasMdk/Types/Structure.h>
#include <top/inner/Colour.h>
#include <top/inner/Ints.h>
#include <top/inner/Label.h>
#include <top/inner/Matrix.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

/// Top namespace doc.
namespace top
{
/// Inner namespace doc.
///  Second line.
namespace inner
{
/// A payload.
struct Payload
{
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

    Payload() = default;
    ~Payload() = default;
    Payload(const Payload&) = default;
    Payload(Payload&&) = default;
    Payload(::Smp::Bool enabled, ::Smp::Int32 count, ::Smp::Float64 ratio, ::top::inner::Colour colour, ::top::inner::Ints values, ::top::inner::Matrix grid, ::top::inner::Label name, ::Smp::String8 lost, ::Smp::Int32 changing, ::Smp::Duration span, ::Smp::DateTime when, ::Smp::Int8 letter):
    enabled(enabled), count(count), ratio(ratio), colour(colour), values(values), grid(grid), name(name), lost(lost), changing(changing), span(span), when(when), letter(letter) {}
    Payload& operator=(const Payload&) = default;

    static void _Register(::Smp::Publication::ITypeRegistry* registry);
};

/// Universally unique identifier of type Payload.
constexpr ::Smp::Uuid Uuid_Payload { 0x10000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000014U };

template<bool ...Opts>
struct _Payload : public ::TasMdk::Types::StructureType<Opts...>
{
    // the equivalent raw_type
    using raw_type = ::top::inner::Payload;

    // constructor
    _Payload(const std::string& name, const std::string& description,
            Smp::IObject *parent, Smp::ViewKind view,
            const Smp::Publication::ITypeRegistry *type_registry, Smp::Uuid typeUuid,
            const raw_type &default_value = raw_type{}) :
            ::TasMdk::Types::StructureType<Opts...>(name, description, parent, view, type_registry->GetType(typeUuid)),
            /// enabled initialization
            enabled{"enabled", "A flag.", this, view, type_registry, ::Smp::Uuids::Uuid_Bool, default_value.enabled},
            /// count initialization
            count{"count", "", this, view, type_registry, ::Smp::Uuids::Uuid_Int32, default_value.count},
            /// ratio initialization
            ratio{"ratio", "", this, ::Smp::ViewKind::VK_Debug, type_registry, ::Smp::Uuids::Uuid_Float64, default_value.ratio},
            /// colour initialization
            colour{"colour", "", this, view, type_registry, ::top::inner::Uuid_Colour, default_value.colour},
            /// values initialization
            values{"values", "", this, view, type_registry, ::top::inner::Uuid_Ints, default_value.values},
            /// grid initialization
            grid{"grid", "", this, view, type_registry, ::top::inner::Uuid_Matrix, default_value.grid},
            /// name initialization
            name{"name", "", this, view, type_registry, ::top::inner::Uuid_Label, default_value.name},
            /// lost initialization
            lost{"lost", "", this, view, type_registry, ::Smp::Uuids::Uuid_String8, default_value.lost},
            /// changing initialization
            changing{"changing", "", this, view, type_registry, ::Smp::Uuids::Uuid_Int32, default_value.changing},
            /// span initialization
            span{"span", "", this, view, type_registry, ::Smp::Uuids::Uuid_Duration, default_value.span},
            /// when initialization
            when{"when", "", this, view, type_registry, ::Smp::Uuids::Uuid_DateTime, default_value.when},
            /// letter initialization
            letter{"letter", "", this, view, type_registry, ::Smp::Uuids::Uuid_Int8, default_value.letter} {
    }

    // copy operator
    _Payload & operator=(const _Payload &other)
    {
        enabled = other.enabled;
        count = other.count;
        ratio = other.ratio;
        colour = other.colour;
        values = other.values;
        grid = other.grid;
        name = other.name;
        lost = other.lost;
        changing = other.changing;
        span = other.span;
        when = other.when;
        letter = other.letter;
        return *this;
    }

    // copy operator from raw_type
    _Payload & operator=(const raw_type &other)
    {
        enabled = other.enabled;
        count = other.count;
        ratio = other.ratio;
        colour = other.colour;
        values = other.values;
        grid = other.grid;
        name = other.name;
        lost = other.lost;
        changing = other.changing;
        span = other.span;
        when = other.when;
        letter = other.letter;
        return *this;
    }

    // convert to raw_type
    operator raw_type() const noexcept
    {
        return {enabled, count, ratio, colour, values, grid, name, lost, changing, span, when, letter};
    }
    /// A flag.
    typename ::TasMdk::SimpleField<::Smp::Bool>::in<Opts...>::type enabled;
    typename ::TasMdk::SimpleField<::Smp::Int32>::in<Opts...>::transient::type count;
    typename ::TasMdk::SimpleField<::Smp::Float64>::in<Opts...>::input::output::type ratio;
    typename ::TasMdk::SimpleField<::top::inner::Colour>::in<Opts...>::type colour;
    typename ::TasMdk::ArrayField<::top::inner::_Ints>::in<Opts...>::type values;
    typename ::TasMdk::ArrayField<::top::inner::_Matrix>::in<Opts...>::type grid;
    typename ::TasMdk::SimpleField<::top::inner::Label>::in<Opts...>::type name;
    typename ::TasMdk::SimpleField<::Smp::String8>::in<Opts...>::type lost;
    mutable typename ::TasMdk::SimpleField<::Smp::Int32>::in<Opts...>::type changing;
    typename ::TasMdk::SimpleField<::Smp::Duration>::in<Opts...>::type span;
    typename ::TasMdk::SimpleField<::Smp::DateTime>::in<Opts...>::type when;
    typename ::TasMdk::SimpleField<::Smp::Int8>::in<Opts...>::type letter;
};
} // namespace inner
} // namespace top

#endif // TOP_INNER_PAYLOAD_H_
