// -----------------------------------------------------------------------------
// File Name    : SimpleSamples.h
// -----------------------------------------------------------------------------
/// @file n/SimpleSamples.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef N_SIMPLESAMPLES_H_
#define N_SIMPLESAMPLES_H_

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
using SimpleSamples = ::Xsmp::Array<::Smp::Float32, 16>::simple;

/// Universally unique identifier of type SimpleSamples.
inline constexpr ::Smp::Uuid Uuid_SimpleSamples { 0x11111111U, 0x0000U, 0x4000U, 0x8000U, 0x000000000023U };

void _Register_SimpleSamples(::Smp::Publication::ITypeRegistry* registry);
} // namespace n

#endif // N_SIMPLESAMPLES_H_
