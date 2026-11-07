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
#include <TasMdk/Types/String.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

/// Top namespace doc.
namespace top
{
/// Inner namespace doc.
///  Second line.
namespace inner
{
/// A label.
using Label = ::TasMdk::Types::String<1024>;

/// Universally unique identifier of type Label.
constexpr ::Smp::Uuid Uuid_Label { 0x10000000U, 0x0000U, 0x4000U, 0x8000U, 0x00000000000bU };

void _Register_Label(::Smp::Publication::ITypeRegistry* registry);
} // namespace inner
} // namespace top

#endif // TOP_INNER_LABEL_H_
