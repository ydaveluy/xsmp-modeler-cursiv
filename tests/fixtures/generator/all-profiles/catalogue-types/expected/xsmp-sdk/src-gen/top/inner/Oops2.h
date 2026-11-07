// Copyright line one
// Copyright line two
// -----------------------------------------------------------------------------
// File Name    : Oops2.h
// -----------------------------------------------------------------------------
/// @file top/inner/Oops2.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef TOP_INNER_OOPS2_H_
#define TOP_INNER_OOPS2_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <Smp/Publication/ITypeRegistry.h>
#include <top/inner/Oops.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

/// Inner namespace doc.
///  Second line.
namespace top::inner
{
class Oops2: public ::top::inner::Oops
{
    public:
    static void _Register(::Smp::Publication::ITypeRegistry* registry);

    Oops2() = default;
    ~Oops2() noexcept = default;
    Oops2(const Oops2&) = default;
};

/// Universally unique identifier of type Oops2.
inline constexpr ::Smp::Uuid Uuid_Oops2 { 0x10000000U, 0x0000U, 0x4000U, 0x8000U, 0x00000000001cU };
} // namespace top::inner

#endif // TOP_INNER_OOPS2_H_
