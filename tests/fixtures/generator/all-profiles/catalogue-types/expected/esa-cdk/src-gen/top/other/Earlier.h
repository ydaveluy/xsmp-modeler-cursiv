// Copyright line one
// Copyright line two
// -----------------------------------------------------------------------------
// File Name    : Earlier.h
// -----------------------------------------------------------------------------
/// @file top/other/Earlier.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef TOP_OTHER_EARLIER_H_
#define TOP_OTHER_EARLIER_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <Smp/Publication/ITypeRegistry.h>
#include <Smp/ViewKind.h>
#include <top/other/Later2.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

/// Top namespace doc.
namespace top
{
namespace other
{
struct Earlier
{
    ::top::other::Later2 x;

    static void _Register(::Smp::Publication::ITypeRegistry* registry);
};

/// Universally unique identifier of type Earlier.
constexpr ::Smp::Uuid Uuid_Earlier { 0x10000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000022U };
} // namespace other
} // namespace top

#endif // TOP_OTHER_EARLIER_H_
