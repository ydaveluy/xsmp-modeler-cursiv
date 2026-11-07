// Copyright line one
// Copyright line two
// -----------------------------------------------------------------------------
// File Name    : Ints.h
// -----------------------------------------------------------------------------
/// @file top/inner/Ints.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef TOP_INNER_INTS_H_
#define TOP_INNER_INTS_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <Xsmp/Array.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

/// Inner namespace doc.
///  Second line.
namespace top::inner
{
using Ints = ::Xsmp::Array<::Smp::Int32, 3>;

/// Universally unique identifier of type Ints.
inline constexpr ::Smp::Uuid Uuid_Ints { 0x10000000U, 0x0000U, 0x4000U, 0x8000U, 0x00000000000dU };

void _Register_Ints(::Smp::Publication::ITypeRegistry* registry);
} // namespace top::inner

#endif // TOP_INNER_INTS_H_
