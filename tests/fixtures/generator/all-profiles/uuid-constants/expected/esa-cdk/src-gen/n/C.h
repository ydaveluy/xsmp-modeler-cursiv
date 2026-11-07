// -----------------------------------------------------------------------------
// File Name    : C.h
// -----------------------------------------------------------------------------
/// @file n/C.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef N_C_H_
#define N_C_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <Smp/Publication/ITypeRegistry.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace n
{
class C
{
    public:
    static void _Register(::Smp::Publication::ITypeRegistry* registry);

    C() = default;
    ~C() noexcept = default;
    C(const C&) = default;
};

/// Universally unique identifier of type C.
constexpr ::Smp::Uuid Uuid_C { 0x11111111U, 0x2222U, 0x3333U, 0x4444U, 0x555555555555U };
} // namespace n

#endif // N_C_H_
