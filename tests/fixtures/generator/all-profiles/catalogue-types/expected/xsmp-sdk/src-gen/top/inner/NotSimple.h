// Copyright line one
// Copyright line two
// -----------------------------------------------------------------------------
// File Name    : NotSimple.h
// -----------------------------------------------------------------------------
/// @file top/inner/NotSimple.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef TOP_INNER_NOTSIMPLE_H_
#define TOP_INNER_NOTSIMPLE_H_

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
using NotSimple = ::Xsmp::Array<::Smp::Float64, 2>;

/// Universally unique identifier of type NotSimple.
inline constexpr ::Smp::Uuid Uuid_NotSimple { 0x10000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000010U };

void _Register_NotSimple(::Smp::Publication::ITypeRegistry* registry);
} // namespace top::inner

#endif // TOP_INNER_NOTSIMPLE_H_
