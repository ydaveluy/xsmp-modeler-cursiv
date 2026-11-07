// Copyright line one
// Copyright line two
// -----------------------------------------------------------------------------
// File Name    : Parent.h
// -----------------------------------------------------------------------------
/// @file top/other/Parent.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef TOP_OTHER_PARENT_H_
#define TOP_OTHER_PARENT_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <Smp/Publication/ITypeRegistry.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

/// Top namespace doc.
namespace top
{
namespace other
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
constexpr ::Smp::Uuid Uuid_Parent { 0x10000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000025U };
} // namespace other
} // namespace top

#endif // TOP_OTHER_PARENT_H_
