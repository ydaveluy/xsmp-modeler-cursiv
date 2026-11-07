// -----------------------------------------------------------------------------
// File Name    : Points.h
// -----------------------------------------------------------------------------
/// @file defaults/Points.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef DEFAULTS_POINTS_H_
#define DEFAULTS_POINTS_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <defaults/Point.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <Xsmp/Array.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace defaults
{
using Points = ::Xsmp::Array<::defaults::Point, 2>;

/// Universally unique identifier of type Points.
inline constexpr ::Smp::Uuid Uuid_Points { 0x70000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000006U };

void _Register_Points(::Smp::Publication::ITypeRegistry* registry);
} // namespace defaults

#endif // DEFAULTS_POINTS_H_
