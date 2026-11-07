// -----------------------------------------------------------------------------
// File Name    : Samples.h
// -----------------------------------------------------------------------------
/// @file n/Samples.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef N_SAMPLES_H_
#define N_SAMPLES_H_

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
using Samples = ::Xsmp::Array<::Smp::Float32, 16>;

/// Universally unique identifier of type Samples.
inline constexpr ::Smp::Uuid Uuid_Samples { 0x11111111U, 0x0000U, 0x4000U, 0x8000U, 0x000000000022U };

void _Register_Samples(::Smp::Publication::ITypeRegistry* registry);
} // namespace n

#endif // N_SAMPLES_H_
