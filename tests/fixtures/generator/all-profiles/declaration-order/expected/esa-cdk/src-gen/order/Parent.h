// -----------------------------------------------------------------------------
// File Name    : Parent.h
// -----------------------------------------------------------------------------
/// @file order/Parent.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef ORDER_PARENT_H_
#define ORDER_PARENT_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <Smp/Publication/ITypeRegistry.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace order
{
class Parent
{
    public:
    static void _Register(::Smp::Publication::ITypeRegistry* registry);

    Parent() = default;
    ~Parent() noexcept = default;
    Parent(const Parent&) = default;
};

/// Universally unique identifier of type Parent.
constexpr ::Smp::Uuid Uuid_Parent { 0x83000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000007U };
} // namespace order

#endif // ORDER_PARENT_H_
