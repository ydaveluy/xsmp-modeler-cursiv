// -----------------------------------------------------------------------------
// File Name    : Whole.h
// -----------------------------------------------------------------------------
/// @file n/Whole.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef N_WHOLE_H_
#define N_WHOLE_H_

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
/// Universally unique identifier of type Whole.
inline constexpr ::Smp::Uuid Uuid_Whole { 0x11111111U, 0x0000U, 0x4000U, 0x8000U, 0x000000000012U };
using Whole = ::Smp::UInt64;
void _Register_Whole(::Smp::Publication::ITypeRegistry* registry);
} // namespace n

#endif // N_WHOLE_H_
