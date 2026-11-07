// -----------------------------------------------------------------------------
// File Name    : Pct.h
// -----------------------------------------------------------------------------
/// @file n/Pct.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef N_PCT_H_
#define N_PCT_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/ITypeRegistry.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace n
{
/// Universally unique identifier of type Pct.
inline constexpr ::Smp::Uuid Uuid_Pct { 0x11111111U, 0x2222U, 0x3333U, 0x4444U, 0x555555555555U };
/// A percentage.
/// @unit %
using Pct = ::Smp::Int32;
void _Register_Pct(::Smp::Publication::ITypeRegistry* registry);
} // namespace n

#endif // N_PCT_H_
