// Copyright line one
// Copyright line two
// -----------------------------------------------------------------------------
// File Name    : Bare.h
// -----------------------------------------------------------------------------
/// @file top/inner/Bare.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef TOP_INNER_BARE_H_
#define TOP_INNER_BARE_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <Smp/Publication/ITypeRegistry.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

/// Inner namespace doc.
///  Second line.
namespace top::inner
{
class Bare
{
    public:
    static void _Register(::Smp::Publication::ITypeRegistry* registry);

    Bare(const Bare&) = default;
};

/// Universally unique identifier of type Bare.
inline constexpr ::Smp::Uuid Uuid_Bare { 0x10000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000019U };
} // namespace top::inner

#endif // TOP_INNER_BARE_H_
