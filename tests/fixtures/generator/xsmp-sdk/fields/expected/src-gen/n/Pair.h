// -----------------------------------------------------------------------------
// File Name    : Pair.h
// -----------------------------------------------------------------------------
/// @file n/Pair.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef N_PAIR_H_
#define N_PAIR_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <n/Single.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <Xsmp/Array.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace n
{
using Pair = ::Xsmp::Array<::n::Single, 2>;

/// Universally unique identifier of type Pair.
inline constexpr ::Smp::Uuid Uuid_Pair { 0x11111111U, 0x0000U, 0x4000U, 0x8000U, 0x000000000086U };

void _Register_Pair(::Smp::Publication::ITypeRegistry* registry);
} // namespace n

#endif // N_PAIR_H_
