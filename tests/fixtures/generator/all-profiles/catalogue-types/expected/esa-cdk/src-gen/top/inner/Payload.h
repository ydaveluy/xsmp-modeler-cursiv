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
} // namespace inner
} // namespace top

#endif // TOP_INNER_PAYLOAD_H_
