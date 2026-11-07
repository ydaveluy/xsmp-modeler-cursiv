// -----------------------------------------------------------------------------
// File Name    : Labels.h
// -----------------------------------------------------------------------------
/// @file n/Labels.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef N_LABELS_H_
#define N_LABELS_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <esa/ecss/smp/cdk/Array.h>
#include <n/Label.h>
#include <Smp/Publication/ITypeRegistry.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace n
{
using Labels = ::esa::ecss::smp::cdk::Array<::n::Label, 2>;

/// Universally unique identifier of type Labels.
constexpr ::Smp::Uuid Uuid_Labels { 0x11111111U, 0x0000U, 0x4000U, 0x8000U, 0x000000000112U };

void _Register_Labels(::Smp::Publication::ITypeRegistry* registry);
} // namespace n

#endif // N_LABELS_H_
