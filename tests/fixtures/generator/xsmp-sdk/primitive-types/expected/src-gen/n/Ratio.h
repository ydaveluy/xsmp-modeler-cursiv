// -----------------------------------------------------------------------------
// File Name    : Ratio.h
// -----------------------------------------------------------------------------
/// @file n/Ratio.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef N_RATIO_H_
#define N_RATIO_H_

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
/// Universally unique identifier of type Ratio.
inline constexpr ::Smp::Uuid Uuid_Ratio { 0x11111111U, 0x0000U, 0x4000U, 0x8000U, 0x000000000017U };
using Ratio = ::Smp::Float64;
void _Register_Ratio(::Smp::Publication::ITypeRegistry* registry);
} // namespace n

#endif // N_RATIO_H_
