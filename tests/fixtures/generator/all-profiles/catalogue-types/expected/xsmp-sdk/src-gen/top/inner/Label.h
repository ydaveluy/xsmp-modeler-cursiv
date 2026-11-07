// Copyright line one
// Copyright line two
// -----------------------------------------------------------------------------
// File Name    : Label.h
// -----------------------------------------------------------------------------
/// @file top/inner/Label.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef TOP_INNER_LABEL_H_
#define TOP_INNER_LABEL_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <Smp/Publication/ITypeRegistry.h>
#include <Xsmp/String.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

/// Inner namespace doc.
///  Second line.
namespace top::inner
{
/// A label.
using Label = ::Xsmp::String<1'024>;

/// Universally unique identifier of type Label.
inline constexpr ::Smp::Uuid Uuid_Label { 0x10000000U, 0x0000U, 0x4000U, 0x8000U, 0x00000000000bU };

void _Register_Label(::Smp::Publication::ITypeRegistry* registry);
} // namespace top::inner

#endif // TOP_INNER_LABEL_H_
