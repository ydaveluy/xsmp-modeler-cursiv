// Copyright line one
// Copyright line two
// -----------------------------------------------------------------------------
// File Name    : Uses.h
// -----------------------------------------------------------------------------
/// @file top/other/Uses.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef TOP_OTHER_USES_H_
#define TOP_OTHER_USES_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <Smp/ViewKind.h>
#include <top/inner/Colour.h>
#include <top/inner/Payload.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

/// Top namespace doc.
namespace top
{
namespace other
{
struct Uses
{
    static constexpr ::Smp::Int32 V{::top::inner::Payload::Answer + 1};
    ::top::inner::Payload p;
    ::top::inner::Colour c{::top::inner::Colour::Red};

    Uses() = default;
    ~Uses() = default;
    Uses(const Uses&) = default;
    Uses(Uses&&) = default;
    Uses(::top::inner::Payload p, ::top::inner::Colour c):
    p(p), c(c) {}
    Uses& operator=(const Uses&) = default;

    static void _Register(::Smp::Publication::ITypeRegistry* registry);
};

/// Universally unique identifier of type Uses.
constexpr ::Smp::Uuid Uuid_Uses { 0x10000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000020U };
} // namespace other
} // namespace top

#endif // TOP_OTHER_USES_H_
