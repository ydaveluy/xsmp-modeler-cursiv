// -----------------------------------------------------------------------------
// File Name    : Count.h
// -----------------------------------------------------------------------------
/// @file n/Count.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef N_COUNT_H_
#define N_COUNT_H_

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
/// Universally unique identifier of type Count.
inline constexpr ::Smp::Uuid Uuid_Count { 0x11111111U, 0x0000U, 0x4000U, 0x8000U, 0x000000000016U };
/// @unit kg\m"s
using Count = ::Smp::Int32;
void _Register_Count(::Smp::Publication::ITypeRegistry* registry);
} // namespace n

#endif // N_COUNT_H_
