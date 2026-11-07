// Copyright line one
// Copyright line two
// -----------------------------------------------------------------------------
// File Name    : Kid.h
// -----------------------------------------------------------------------------
/// @file top/other/Kid.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef TOP_OTHER_KID_H_
#define TOP_OTHER_KID_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <Smp/Publication/ITypeRegistry.h>
#include <top/other/Parent.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

/// Top namespace doc.
namespace top
{
namespace other
{
/// Derived declared before its base.
class Kid: public ::top::other::Parent
{
    public:
    static void _Register(::Smp::Publication::ITypeRegistry* registry);

    Kid() = default;
    ~Kid() noexcept = default;
    Kid(const Kid&) = default;
};

/// Universally unique identifier of type Kid.
constexpr ::Smp::Uuid Uuid_Kid { 0x10000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000024U };
} // namespace other
} // namespace top

#endif // TOP_OTHER_KID_H_
