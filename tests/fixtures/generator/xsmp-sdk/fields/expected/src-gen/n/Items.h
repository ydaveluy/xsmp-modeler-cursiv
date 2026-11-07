// -----------------------------------------------------------------------------
// File Name    : Items.h
// -----------------------------------------------------------------------------
/// @file n/Items.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef N_ITEMS_H_
#define N_ITEMS_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <n/WithOutput.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <Xsmp/Array.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace n
{
using Items = ::Xsmp::Array<::n::WithOutput, 2>;

/// Universally unique identifier of type Items.
inline constexpr ::Smp::Uuid Uuid_Items { 0x11111111U, 0x0000U, 0x4000U, 0x8000U, 0x000000000083U };

void _Register_Items(::Smp::Publication::ITypeRegistry* registry);
} // namespace n

#endif // N_ITEMS_H_
