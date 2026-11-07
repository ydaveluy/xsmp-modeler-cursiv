// -----------------------------------------------------------------------------
// File Name    : Matrix.h
// -----------------------------------------------------------------------------
/// @file defaults/Matrix.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef DEFAULTS_MATRIX_H_
#define DEFAULTS_MATRIX_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <defaults/Row.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <Xsmp/Array.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace defaults
{
using Matrix = ::Xsmp::Array<::defaults::Row, 2>;

/// Universally unique identifier of type Matrix.
inline constexpr ::Smp::Uuid Uuid_Matrix { 0x70000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000002U };

void _Register_Matrix(::Smp::Publication::ITypeRegistry* registry);
} // namespace defaults

#endif // DEFAULTS_MATRIX_H_
