// The attribute is only metadata: it changes nothing in the generated code.
// -----------------------------------------------------------------------------
// File Name    : Plain.h
// -----------------------------------------------------------------------------
/// @file n/Plain.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef N_PLAIN_H_
#define N_PLAIN_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <Smp/Publication/ITypeRegistry.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace n
{
class Plain
{
    public:
    static void _Register(::Smp::Publication::ITypeRegistry* registry);

    Plain() = default;
    ~Plain() noexcept = default;
    Plain(const Plain&) = default;
};

/// Universally unique identifier of type Plain.
inline constexpr ::Smp::Uuid Uuid_Plain { 0x11111111U, 0x0000U, 0x4000U, 0x8000U, 0x0000000000d2U };
} // namespace n

#endif // N_PLAIN_H_
