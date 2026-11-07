// -----------------------------------------------------------------------------
// File Name    : Point.h
// -----------------------------------------------------------------------------
/// @file defaults/Point.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef DEFAULTS_POINT_H_
#define DEFAULTS_POINT_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <Smp/ViewKind.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace defaults
{
struct Point
{
    ::Smp::Int32 x;
    ::Smp::Int32 y;

    static void _Register(::Smp::Publication::ITypeRegistry* registry);
};

/// Universally unique identifier of type Point.
constexpr ::Smp::Uuid Uuid_Point { 0x70000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000005U };
} // namespace defaults

#endif // DEFAULTS_POINT_H_
