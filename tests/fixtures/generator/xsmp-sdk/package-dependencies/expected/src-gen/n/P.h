// -----------------------------------------------------------------------------
// File Name    : P.h
// -----------------------------------------------------------------------------
/// @file n/P.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef N_P_H_
#define N_P_H_

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
/// Universally unique identifier of type P.
inline constexpr ::Smp::Uuid Uuid_P { 0x11111111U, 0x0000U, 0x4000U, 0x8000U, 0x000000000001U };
using P = ::Smp::Int32;
void _Register_P(::Smp::Publication::ITypeRegistry* registry);
} // namespace n

#endif // N_P_H_
