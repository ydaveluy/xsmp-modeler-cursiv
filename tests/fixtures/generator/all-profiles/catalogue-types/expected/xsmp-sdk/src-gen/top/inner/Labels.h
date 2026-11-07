// Copyright line one
// Copyright line two
// -----------------------------------------------------------------------------
// File Name    : Labels.h
// -----------------------------------------------------------------------------
/// @file top/inner/Labels.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef TOP_INNER_LABELS_H_
#define TOP_INNER_LABELS_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <Smp/Publication/ITypeRegistry.h>
#include <top/inner/Label.h>
#include <Xsmp/Array.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

/// Inner namespace doc.
///  Second line.
namespace top::inner
{
using Labels = ::Xsmp::Array<::top::inner::Label, 2>;

/// Universally unique identifier of type Labels.
inline constexpr ::Smp::Uuid Uuid_Labels { 0x10000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000011U };

void _Register_Labels(::Smp::Publication::ITypeRegistry* registry);
} // namespace top::inner

#endif // TOP_INNER_LABELS_H_
