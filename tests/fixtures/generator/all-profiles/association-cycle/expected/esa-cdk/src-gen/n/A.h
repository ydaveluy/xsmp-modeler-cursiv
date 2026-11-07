// -----------------------------------------------------------------------------
// File Name    : A.h
// -----------------------------------------------------------------------------
/// @file n/A.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef N_A_H_
#define N_A_H_

// ----------------------------------------------------------------------------
// --------------------------- Forward Declarations ---------------------------
// ----------------------------------------------------------------------------

namespace n
{
class B;
} // namespace n

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

    private:
    ::n::B* b;
};

/// Universally unique identifier of type A.
constexpr ::Smp::Uuid Uuid_A { 0x11111111U, 0x2222U, 0x3333U, 0x4444U, 0x555555555555U };
} // namespace n

#endif // N_A_H_
