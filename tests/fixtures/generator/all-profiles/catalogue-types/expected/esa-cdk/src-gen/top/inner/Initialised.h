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

/// Top namespace doc.
namespace top
{
/// Inner namespace doc.
///  Second line.
namespace inner
{
struct Initialised
{
    ::top::inner::Pair byName{ /* .x = */1, /* .y = */2};
    ::top::inner::Pair byPosition{ 3, 4};

    Initialised() = default;
    ~Initialised() = default;
    Initialised(const Initialised&) = default;
    Initialised(Initialised&&) = default;
    Initialised(::top::inner::Pair byName, ::top::inner::Pair byPosition):
    byName(byName), byPosition(byPosition) {}
    Initialised& operator=(const Initialised&) = default;

    static void _Register(::Smp::Publication::ITypeRegistry* registry);
};

/// Universally unique identifier of type Initialised.
constexpr ::Smp::Uuid Uuid_Initialised { 0x10000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000028U };
} // namespace inner
} // namespace top

#endif // TOP_INNER_INITIALISED_H_
