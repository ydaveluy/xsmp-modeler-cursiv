// -----------------------------------------------------------------------------
// File Name    : D.h
// -----------------------------------------------------------------------------
/// @file n/D.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef N_D_H_
#define N_D_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <Smp/Publication/ITypeRegistry.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace n
{
class D
{
    public:
    static void _Register(::Smp::Publication::ITypeRegistry* registry);

    D() = default;
    D(const D&) = default;
};

/// Universally unique identifier of type D.
inline constexpr ::Smp::Uuid Uuid_D { 0x11111111U, 0x0000U, 0x4000U, 0x8000U, 0x0000000000c4U };
} // namespace n

#endif // N_D_H_
