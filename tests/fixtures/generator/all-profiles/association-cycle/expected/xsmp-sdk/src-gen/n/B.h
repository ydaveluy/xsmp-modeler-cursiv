// -----------------------------------------------------------------------------
// File Name    : B.h
// -----------------------------------------------------------------------------
/// @file n/B.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef N_B_H_
#define N_B_H_

// ----------------------------------------------------------------------------
// --------------------------- Forward Declarations ---------------------------
// ----------------------------------------------------------------------------

namespace n
{
class A;
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
class B
{
    public:
    static void _Register(::Smp::Publication::ITypeRegistry* registry);

    B() = default;
    ~B() noexcept = default;
    B(const B&) = default;

    private:
    ::n::A* a;
};

/// Universally unique identifier of type B.
inline constexpr ::Smp::Uuid Uuid_B { 0xaaaaaaaaU, 0xbbbbU, 0xccccU, 0xddddU, 0xeeeeeeeeeeeeU };
} // namespace n

#endif // N_B_H_
