// -----------------------------------------------------------------------------
// File Name    : Small.h
// -----------------------------------------------------------------------------
/// @file n/Small.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef N_SMALL_H_
#define N_SMALL_H_

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
/// Universally unique identifier of type Small.
inline constexpr ::Smp::Uuid Uuid_Small { 0x11111111U, 0x0000U, 0x4000U, 0x8000U, 0x000000000014U };
using Small = ::Smp::UInt64;
void _Register_Small(::Smp::Publication::ITypeRegistry* registry);
} // namespace n

#endif // N_SMALL_H_
