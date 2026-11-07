// -----------------------------------------------------------------------------
// File Name    : Row.h
// -----------------------------------------------------------------------------
/// @file n/Row.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef N_ROW_H_
#define N_ROW_H_

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
using Row = ::Xsmp::Array<::Smp::Int32, 2>;

/// Universally unique identifier of type Row.
inline constexpr ::Smp::Uuid Uuid_Row { 0x11111111U, 0x0000U, 0x4000U, 0x8000U, 0x000000000087U };

void _Register_Row(::Smp::Publication::ITypeRegistry* registry);
} // namespace n

#endif // N_ROW_H_
