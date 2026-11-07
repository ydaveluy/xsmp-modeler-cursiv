// -----------------------------------------------------------------------------
// File Name    : Outer.h
// -----------------------------------------------------------------------------
/// @file order/Outer.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef ORDER_OUTER_H_
#define ORDER_OUTER_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <order/Inner.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <Xsmp/Array.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace order
{
using Outer = ::Xsmp::Array<::order::Inner, 2>;

/// Universally unique identifier of type Outer.
inline constexpr ::Smp::Uuid Uuid_Outer { 0x83000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000001U };

void _Register_Outer(::Smp::Publication::ITypeRegistry* registry);
} // namespace order

#endif // ORDER_OUTER_H_
