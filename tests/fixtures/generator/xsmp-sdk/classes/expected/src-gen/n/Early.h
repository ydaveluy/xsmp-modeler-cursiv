// -----------------------------------------------------------------------------
// File Name    : Early.h
// -----------------------------------------------------------------------------
/// @file n/Early.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef N_EARLY_H_
#define N_EARLY_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <Smp/Publication/ITypeRegistry.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace n
{
class Early
{
    public:
    static void _Register(::Smp::Publication::ITypeRegistry* registry);

    Early() = default;
    ~Early() noexcept = default;
    Early(const Early&) = default;
};

/// Universally unique identifier of type Early.
inline constexpr ::Smp::Uuid Uuid_Early { 0x11111111U, 0x0000U, 0x4000U, 0x8000U, 0x000000000044U };
} // namespace n

#endif // N_EARLY_H_
