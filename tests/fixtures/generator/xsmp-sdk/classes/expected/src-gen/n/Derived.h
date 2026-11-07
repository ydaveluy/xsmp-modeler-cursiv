// -----------------------------------------------------------------------------
// File Name    : Derived.h
// -----------------------------------------------------------------------------
/// @file n/Derived.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef N_DERIVED_H_
#define N_DERIVED_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <n/Base.h>
#include <Smp/Publication/ITypeRegistry.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace n
{
class Derived: public ::n::Base
{
    public:
    static void _Register(::Smp::Publication::ITypeRegistry* registry);

    Derived() = default;
    ~Derived() noexcept = default;
    Derived(const Derived&) = default;
};

/// Universally unique identifier of type Derived.
inline constexpr ::Smp::Uuid Uuid_Derived { 0x11111111U, 0x0000U, 0x4000U, 0x8000U, 0x000000000042U };
} // namespace n

#endif // N_DERIVED_H_
