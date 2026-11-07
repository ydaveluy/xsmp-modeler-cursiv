// -----------------------------------------------------------------------------
// File Name    : X.h
// -----------------------------------------------------------------------------
/// @file n/X.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef N_X_H_
#define N_X_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <Smp/Publication/ITypeRegistry.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace n
{
class X: public ::Smp::Exception
{
    public:
    static void _Register(::Smp::Publication::ITypeRegistry* registry);

    X() = default;
    ~X() noexcept = default;
    X(const X&) = default;
};

/// Universally unique identifier of type X.
inline constexpr ::Smp::Uuid Uuid_X { 0x11111111U, 0x0000U, 0x4000U, 0x8000U, 0x0000000000c3U };
} // namespace n

#endif // N_X_H_
