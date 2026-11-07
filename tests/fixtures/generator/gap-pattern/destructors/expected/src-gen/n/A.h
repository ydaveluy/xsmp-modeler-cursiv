// -----------------------------------------------------------------------------
// File Name    : A.h
// -----------------------------------------------------------------------------
/// @file n/A.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef N_A_H_
#define N_A_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <Smp/Publication/ITypeRegistry.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace n
{
class A
{
    public:
    static void _Register(::Smp::Publication::ITypeRegistry* registry);

    A() = default;
    ~A() noexcept = default;
    A(const A&) = default;
};

/// Universally unique identifier of type A.
inline constexpr ::Smp::Uuid Uuid_A { 0x11111111U, 0x0000U, 0x4000U, 0x8000U, 0x0000000000c2U };
} // namespace n

#endif // N_A_H_
