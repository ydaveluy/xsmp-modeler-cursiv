// -----------------------------------------------------------------------------
// File Name    : Inner.h
// -----------------------------------------------------------------------------
/// @file order/Inner.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef ORDER_INNER_H_
#define ORDER_INNER_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <order/Value.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <Xsmp/Array.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace order
{
using Inner = ::Xsmp::Array<::order::Value, 2>;

/// Universally unique identifier of type Inner.
inline constexpr ::Smp::Uuid Uuid_Inner { 0x83000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000002U };

void _Register_Inner(::Smp::Publication::ITypeRegistry* registry);
} // namespace order

#endif // ORDER_INNER_H_
