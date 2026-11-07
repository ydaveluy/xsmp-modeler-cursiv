// -----------------------------------------------------------------------------
// File Name    : Label.h
// -----------------------------------------------------------------------------
/// @file n/Label.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef N_LABEL_H_
#define N_LABEL_H_

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
/// A short label.
using Label = ::Xsmp::String<16>;

/// Universally unique identifier of type Label.
inline constexpr ::Smp::Uuid Uuid_Label { 0x11111111U, 0x0000U, 0x4000U, 0x8000U, 0x000000000019U };

void _Register_Label(::Smp::Publication::ITypeRegistry* registry);
} // namespace n

#endif // N_LABEL_H_
