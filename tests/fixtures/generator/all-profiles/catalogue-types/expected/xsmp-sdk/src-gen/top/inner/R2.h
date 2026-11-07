// Copyright line one
// Copyright line two
// -----------------------------------------------------------------------------
// File Name    : R2.h
// -----------------------------------------------------------------------------
/// @file top/inner/R2.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef TOP_INNER_R2_H_
#define TOP_INNER_R2_H_

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
/// Universally unique identifier of type R2.
inline constexpr ::Smp::Uuid Uuid_R2 { 0x10000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000008U };
using R2 = ::Smp::Float64;
void _Register_R2(::Smp::Publication::ITypeRegistry* registry);
} // namespace top::inner

#endif // TOP_INNER_R2_H_
