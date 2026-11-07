// -----------------------------------------------------------------------------
// File Name    : Grid.h
// -----------------------------------------------------------------------------
/// @file n/Grid.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef N_GRID_H_
#define N_GRID_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <n/Row.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <Xsmp/Array.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace n
{
using Grid = ::Xsmp::Array<::n::Row, 2>;

/// Universally unique identifier of type Grid.
inline constexpr ::Smp::Uuid Uuid_Grid { 0x11111111U, 0x0000U, 0x4000U, 0x8000U, 0x000000000088U };

void _Register_Grid(::Smp::Publication::ITypeRegistry* registry);
} // namespace n

#endif // N_GRID_H_
