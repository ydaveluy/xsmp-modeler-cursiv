// Copyright line one
// Copyright line two
// -----------------------------------------------------------------------------
// File Name    : Colour.h
// -----------------------------------------------------------------------------
/// @file top/inner/Colour.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef TOP_INNER_COLOUR_H_
#define TOP_INNER_COLOUR_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <map>
#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <string>

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
/// An enumeration.
enum class Colour: ::Smp::Int32 {
    /// The red one.
    Red = 0,
    /// The green one, it's "fine".
    Green = 1,
    Blue = 0x10
};

/// Universally unique identifier of type Colour.
constexpr ::Smp::Uuid Uuid_Colour { 0x10000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000001U };

void _Register_Colour(::Smp::Publication::ITypeRegistry* registry);

const std::map<Colour, std::string> Colour_name_map = {
    { Colour::Red, "Red" }, { Colour::Green, "Green" }, { Colour::Blue, "Blue" }
};

const std::map<Colour, std::string> Colour_descr_map = {
    { Colour::Red, "The red one." }, { Colour::Green, "The green one, it\'s \"fine\"." }, { Colour::Blue, "" }
};
} // namespace inner
} // namespace top

#endif // TOP_INNER_COLOUR_H_
