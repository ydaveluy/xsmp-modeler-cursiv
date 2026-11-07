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

#include <esa/ecss/smp/cdk/String.h>
#include <Smp/Publication/ITypeRegistry.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace n
{
using Label = ::esa::ecss::smp::cdk::String<8>;

/// Universally unique identifier of type Label.
constexpr ::Smp::Uuid Uuid_Label { 0x11111111U, 0x0000U, 0x4000U, 0x8000U, 0x000000000111U };

void _Register_Label(::Smp::Publication::ITypeRegistry* registry);
} // namespace n

#endif // N_LABEL_H_
