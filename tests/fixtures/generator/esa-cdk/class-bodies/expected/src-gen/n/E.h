// -----------------------------------------------------------------------------
// File Name    : E.h
// -----------------------------------------------------------------------------
/// @file n/E.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef N_E_H_
#define N_E_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <Smp/Publication/ITypeRegistry.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace n
{
class E: public ::Smp::Exception
{
    public:
    static void _Register(::Smp::Publication::ITypeRegistry* registry);

    E() = default;
    ~E() noexcept = default;
    E(const E&) = default;

    void reset();
};

/// Universally unique identifier of type E.
constexpr ::Smp::Uuid Uuid_E { 0x11111111U, 0x0000U, 0x4000U, 0x8000U, 0x000000000132U };
} // namespace n

#endif // N_E_H_
