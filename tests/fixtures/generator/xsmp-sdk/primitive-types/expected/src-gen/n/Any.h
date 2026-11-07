// -----------------------------------------------------------------------------
// File Name    : Any.h
// -----------------------------------------------------------------------------
/// @file n/Any.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef N_ANY_H_
#define N_ANY_H_

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
/// Universally unique identifier of type Any.
inline constexpr ::Smp::Uuid Uuid_Any { 0x11111111U, 0x0000U, 0x4000U, 0x8000U, 0x000000000011U };
using Any = ::Smp::Int32;
void _Register_Any(::Smp::Publication::ITypeRegistry* registry);
} // namespace n

#endif // N_ANY_H_
