// -----------------------------------------------------------------------------
// File Name    : Text.h
// -----------------------------------------------------------------------------
/// @file n/Text.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef N_TEXT_H_
#define N_TEXT_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <Smp/Publication/ITypeRegistry.h>
#include <Xsmp/String.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace n
{
using Text = ::Xsmp::String<4>;

/// Universally unique identifier of type Text.
inline constexpr ::Smp::Uuid Uuid_Text { 0x11111111U, 0x0000U, 0x4000U, 0x8000U, 0x0000000000b4U };

void _Register_Text(::Smp::Publication::ITypeRegistry* registry);
} // namespace n

#endif // N_TEXT_H_
