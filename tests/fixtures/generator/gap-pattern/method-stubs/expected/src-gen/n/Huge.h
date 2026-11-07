// -----------------------------------------------------------------------------
// File Name    : Huge.h
// -----------------------------------------------------------------------------
/// @file n/Huge.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef N_HUGE_H_
#define N_HUGE_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <Xsmp/Array.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace n
{
using Huge = ::Xsmp::Array<::Smp::Int32, 10000000>;

/// Universally unique identifier of type Huge.
inline constexpr ::Smp::Uuid Uuid_Huge { 0x11111111U, 0x0000U, 0x4000U, 0x8000U, 0x0000000000f1U };

void _Register_Huge(::Smp::Publication::ITypeRegistry* registry);
} // namespace n

#endif // N_HUGE_H_
