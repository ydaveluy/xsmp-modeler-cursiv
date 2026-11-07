// -----------------------------------------------------------------------------
// File Name    : Holder.h
// -----------------------------------------------------------------------------
/// @file order/Holder.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef ORDER_HOLDER_H_
#define ORDER_HOLDER_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <order/Value.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <Smp/ViewKind.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace order
{
struct Holder
{
    ::order::Value first;
    ::order::Value second;

    static void _Register(::Smp::Publication::ITypeRegistry* registry);
};

/// Universally unique identifier of type Holder.
constexpr ::Smp::Uuid Uuid_Holder { 0x83000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000004U };
} // namespace order

#endif // ORDER_HOLDER_H_
