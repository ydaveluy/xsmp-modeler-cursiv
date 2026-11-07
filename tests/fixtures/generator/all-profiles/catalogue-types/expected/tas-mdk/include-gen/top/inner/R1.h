// Copyright line one
// Copyright line two
// -----------------------------------------------------------------------------
// File Name    : R1.h
// -----------------------------------------------------------------------------
/// @file top/inner/R1.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef TOP_INNER_R1_H_
#define TOP_INNER_R1_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/ITypeRegistry.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

/// Top namespace doc.
namespace top
{
/// Inner namespace doc.
///  Second line.
namespace inner
{
/// Universally unique identifier of type R1.
constexpr ::Smp::Uuid Uuid_R1 { 0x10000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000007U };
using R1 = ::Smp::Float64;
void _Register_R1(::Smp::Publication::ITypeRegistry* registry);
} // namespace inner
} // namespace top

#endif // TOP_INNER_R1_H_
