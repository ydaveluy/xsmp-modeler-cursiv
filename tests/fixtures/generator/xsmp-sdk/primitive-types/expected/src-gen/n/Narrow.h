// -----------------------------------------------------------------------------
// File Name    : Narrow.h
// -----------------------------------------------------------------------------
/// @file n/Narrow.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef N_NARROW_H_
#define N_NARROW_H_

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
/// Universally unique identifier of type Narrow.
inline constexpr ::Smp::Uuid Uuid_Narrow { 0x11111111U, 0x0000U, 0x4000U, 0x8000U, 0x000000000015U };
using Narrow = ::Smp::UInt32;
void _Register_Narrow(::Smp::Publication::ITypeRegistry* registry);
} // namespace n

#endif // N_NARROW_H_
