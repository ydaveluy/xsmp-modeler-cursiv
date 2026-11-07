// -----------------------------------------------------------------------------
// File Name    : Hostile.h
// -----------------------------------------------------------------------------
/// @file n/Hostile.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef N_HOSTILE_H_
#define N_HOSTILE_H_

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
/// Universally unique identifier of type Hostile.
inline constexpr ::Smp::Uuid Uuid_Hostile { 0x11111111U, 0x0000U, 0x4000U, 0x8000U, 0x000000000018U };
/// @unit kg\m"s
using Hostile = ::Smp::Float64;
void _Register_Hostile(::Smp::Publication::ITypeRegistry* registry);
} // namespace n

#endif // N_HOSTILE_H_
