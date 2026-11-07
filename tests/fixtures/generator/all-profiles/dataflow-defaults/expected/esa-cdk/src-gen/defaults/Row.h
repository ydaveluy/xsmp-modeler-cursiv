// -----------------------------------------------------------------------------
// File Name    : Row.h
// -----------------------------------------------------------------------------
/// @file defaults/Row.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef DEFAULTS_ROW_H_
#define DEFAULTS_ROW_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <esa/ecss/smp/cdk/Array.h>
#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/ITypeRegistry.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace defaults
{
using Row = ::esa::ecss::smp::cdk::Array<::Smp::Int32, 2>;

/// Universally unique identifier of type Row.
constexpr ::Smp::Uuid Uuid_Row { 0x70000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000001U };

void _Register_Row(::Smp::Publication::ITypeRegistry* registry);
} // namespace defaults

#endif // DEFAULTS_ROW_H_
