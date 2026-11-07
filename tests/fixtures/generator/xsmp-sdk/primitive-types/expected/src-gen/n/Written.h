// -----------------------------------------------------------------------------
// File Name    : Written.h
// -----------------------------------------------------------------------------
/// @file n/Written.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef N_WRITTEN_H_
#define N_WRITTEN_H_

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
/// Universally unique identifier of type Written.
inline constexpr ::Smp::Uuid Uuid_Written { 0x11111111U, 0x0000U, 0x4000U, 0x8000U, 0x000000000013U };
using Written = ::Smp::UInt64;
void _Register_Written(::Smp::Publication::ITypeRegistry* registry);
} // namespace n

#endif // N_WRITTEN_H_
