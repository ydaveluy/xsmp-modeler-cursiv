// -----------------------------------------------------------------------------
// File Name    : InvalidMode.h
// -----------------------------------------------------------------------------
/// @file n/InvalidMode.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef N_INVALIDMODE_H_
#define N_INVALIDMODE_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/ITypeRegistry.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace n
{
class InvalidMode: public ::Smp::Exception
{
    public:
    static void _Register(::Smp::Publication::ITypeRegistry* registry);

    InvalidMode() = default;
    ~InvalidMode() noexcept = default;
    InvalidMode(const InvalidMode&) = default;

    private:
    ::Smp::String8 reason;
};

/// Universally unique identifier of type InvalidMode.
inline constexpr ::Smp::Uuid Uuid_InvalidMode { 0x11111111U, 0x0000U, 0x4000U, 0x8000U, 0x000000000046U };
} // namespace n

#endif // N_INVALIDMODE_H_
