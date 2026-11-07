// -----------------------------------------------------------------------------
// File Name    : Child.h
// -----------------------------------------------------------------------------
/// @file order/Child.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef ORDER_CHILD_H_
#define ORDER_CHILD_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <order/Parent.h>
#include <Smp/Publication/ITypeRegistry.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace order
{
class Child: public ::order::Parent
{
    public:
    static void _Register(::Smp::Publication::ITypeRegistry* registry);

    Child() = default;
    ~Child() noexcept = default;
    Child(const Child&) = default;
};

/// Universally unique identifier of type Child.
inline constexpr ::Smp::Uuid Uuid_Child { 0x83000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000003U };
} // namespace order

#endif // ORDER_CHILD_H_
