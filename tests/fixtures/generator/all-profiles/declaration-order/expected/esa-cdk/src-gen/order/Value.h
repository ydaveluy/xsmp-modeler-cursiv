// -----------------------------------------------------------------------------
// File Name    : Value.h
// -----------------------------------------------------------------------------
/// @file order/Value.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef ORDER_VALUE_H_
#define ORDER_VALUE_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <Smp/ViewKind.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace order
{
struct Value
{
    ::Smp::Int32 value;

    static void _Register(::Smp::Publication::ITypeRegistry* registry);
};

/// Universally unique identifier of type Value.
constexpr ::Smp::Uuid Uuid_Value { 0x83000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000006U };
} // namespace order

#endif // ORDER_VALUE_H_
