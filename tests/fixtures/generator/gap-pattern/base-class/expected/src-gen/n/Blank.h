// The attribute is only metadata: it changes nothing in the generated code.
// -----------------------------------------------------------------------------
// File Name    : Blank.h
// -----------------------------------------------------------------------------
/// @file n/Blank.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef N_BLANK_H_
#define N_BLANK_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <Smp/Publication/ITypeRegistry.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace n
{
class Blank
{
    public:
    static void _Register(::Smp::Publication::ITypeRegistry* registry);

    Blank() = default;
    ~Blank() noexcept = default;
    Blank(const Blank&) = default;
};

/// Universally unique identifier of type Blank.
inline constexpr ::Smp::Uuid Uuid_Blank { 0x11111111U, 0x0000U, 0x4000U, 0x8000U, 0x0000000000d7U };
} // namespace n

#endif // N_BLANK_H_
