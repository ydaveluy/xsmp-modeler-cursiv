// -----------------------------------------------------------------------------
// File Name    : Name.h
// -----------------------------------------------------------------------------
/// @file n/Name.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef N_NAME_H_
#define N_NAME_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <Smp/Publication/ITypeRegistry.h>
#include <Xsmp/String.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace n
{
using Name = ::Xsmp::String<8>;

/// Universally unique identifier of type Name.
inline constexpr ::Smp::Uuid Uuid_Name { 0x11111111U, 0x0000U, 0x4000U, 0x8000U, 0x000000000094U };

void _Register_Name(::Smp::Publication::ITypeRegistry* registry);
} // namespace n

#endif // N_NAME_H_
