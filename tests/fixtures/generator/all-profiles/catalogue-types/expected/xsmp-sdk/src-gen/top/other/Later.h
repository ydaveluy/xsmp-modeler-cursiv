// Copyright line one
// Copyright line two
// -----------------------------------------------------------------------------
// File Name    : Later.h
// -----------------------------------------------------------------------------
/// @file top/other/Later.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef TOP_OTHER_LATER_H_
#define TOP_OTHER_LATER_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <Smp/Publication/ITypeRegistry.h>
#include <top/other/Earlier.h>
#include <Xsmp/Array.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace top::other
{
using Later = ::Xsmp::Array<::top::other::Earlier, 2>;

/// Universally unique identifier of type Later.
inline constexpr ::Smp::Uuid Uuid_Later { 0x10000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000021U };

void _Register_Later(::Smp::Publication::ITypeRegistry* registry);
} // namespace top::other

#endif // TOP_OTHER_LATER_H_
