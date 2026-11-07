// Copyright line one
// Copyright line two
// -----------------------------------------------------------------------------
// File Name    : R4.h
// -----------------------------------------------------------------------------
/// @file top/inner/R4.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef TOP_INNER_R4_H_
#define TOP_INNER_R4_H_

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
/// Universally unique identifier of type R4.
constexpr ::Smp::Uuid Uuid_R4 { 0x10000000U, 0x0000U, 0x4000U, 0x8000U, 0x00000000000aU };
using R4 = ::Smp::Float64;
void _Register_R4(::Smp::Publication::ITypeRegistry* registry);
} // namespace inner
} // namespace top

#endif // TOP_INNER_R4_H_
