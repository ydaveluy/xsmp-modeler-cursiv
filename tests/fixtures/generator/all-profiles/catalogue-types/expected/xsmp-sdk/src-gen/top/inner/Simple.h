// Copyright line one
// Copyright line two
// -----------------------------------------------------------------------------
// File Name    : Simple.h
// -----------------------------------------------------------------------------
/// @file top/inner/Simple.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef TOP_INNER_SIMPLE_H_
#define TOP_INNER_SIMPLE_H_

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
using Simple = ::Xsmp::Array<::Smp::Bool, 2>::simple;

/// Universally unique identifier of type Simple.
inline constexpr ::Smp::Uuid Uuid_Simple { 0x10000000U, 0x0000U, 0x4000U, 0x8000U, 0x00000000000fU };

void _Register_Simple(::Smp::Publication::ITypeRegistry* registry);
} // namespace top::inner

#endif // TOP_INNER_SIMPLE_H_
