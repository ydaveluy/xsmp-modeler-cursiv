// Copyright line one
// Copyright line two
// -----------------------------------------------------------------------------
// File Name    : Plain.h
// -----------------------------------------------------------------------------
/// @file top/inner/Plain.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef TOP_INNER_PLAIN_H_
#define TOP_INNER_PLAIN_H_

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
/// Universally unique identifier of type Plain.
inline constexpr ::Smp::Uuid Uuid_Plain { 0x10000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000003U };
using Plain = ::Smp::Int32;
void _Register_Plain(::Smp::Publication::ITypeRegistry* registry);
} // namespace top::inner

#endif // TOP_INNER_PLAIN_H_
