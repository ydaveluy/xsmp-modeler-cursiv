// -----------------------------------------------------------------------------
// File Name    : Late.h
// -----------------------------------------------------------------------------
/// @file n/Late.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef N_LATE_H_
#define N_LATE_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <n/Early.h>
#include <Smp/Publication/ITypeRegistry.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace n
{
class Late: public ::n::Early
{
    public:
    static void _Register(::Smp::Publication::ITypeRegistry* registry);

    Late() = default;
    ~Late() noexcept = default;
    Late(const Late&) = default;
};

/// Universally unique identifier of type Late.
inline constexpr ::Smp::Uuid Uuid_Late { 0x11111111U, 0x0000U, 0x4000U, 0x8000U, 0x000000000043U };
} // namespace n

#endif // N_LATE_H_
