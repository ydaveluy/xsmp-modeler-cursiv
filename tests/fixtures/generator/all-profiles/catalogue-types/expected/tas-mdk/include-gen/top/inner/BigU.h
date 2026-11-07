// Copyright line one
// Copyright line two
// -----------------------------------------------------------------------------
// File Name    : BigU.h
// -----------------------------------------------------------------------------
/// @file top/inner/BigU.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef TOP_INNER_BIGU_H_
#define TOP_INNER_BIGU_H_

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
/// Universally unique identifier of type BigU.
constexpr ::Smp::Uuid Uuid_BigU { 0x10000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000004U };
using BigU = ::Smp::UInt64;
void _Register_BigU(::Smp::Publication::ITypeRegistry* registry);
} // namespace inner
} // namespace top

#endif // TOP_INNER_BIGU_H_
