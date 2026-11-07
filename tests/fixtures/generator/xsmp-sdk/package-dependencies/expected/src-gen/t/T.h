// -----------------------------------------------------------------------------
// File Name    : T.h
// -----------------------------------------------------------------------------
/// @file t/T.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef T_T_H_
#define T_T_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/ITypeRegistry.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace t
{
/// Universally unique identifier of type T.
inline constexpr ::Smp::Uuid Uuid_T { 0x11111111U, 0x0000U, 0x4000U, 0x8000U, 0x000000000002U };
using T = ::Smp::Int32;
void _Register_T(::Smp::Publication::ITypeRegistry* registry);
} // namespace t

#endif // T_T_H_
