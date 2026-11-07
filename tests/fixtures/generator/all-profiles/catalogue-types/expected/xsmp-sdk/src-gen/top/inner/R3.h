// Copyright line one
// Copyright line two
// -----------------------------------------------------------------------------
// File Name    : R3.h
// -----------------------------------------------------------------------------
/// @file top/inner/R3.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef TOP_INNER_R3_H_
#define TOP_INNER_R3_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/ITypeRegistry.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

/// Inner namespace doc.
///  Second line.
namespace top::inner
{
/// Universally unique identifier of type R3.
inline constexpr ::Smp::Uuid Uuid_R3 { 0x10000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000009U };
using R3 = ::Smp::Float64;
void _Register_R3(::Smp::Publication::ITypeRegistry* registry);
} // namespace top::inner

#endif // TOP_INNER_R3_H_
