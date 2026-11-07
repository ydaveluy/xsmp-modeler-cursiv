// Copyright line one
// Copyright line two
// -----------------------------------------------------------------------------
// File Name    : Labels.cpp
// -----------------------------------------------------------------------------
/// @file top/inner/Labels.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <Smp/Publication/IArrayType.h>
#include <top/inner/Labels.h>

/// Top namespace doc.
namespace top
{
/// Inner namespace doc.
///  Second line.
namespace inner
{
void _Register_Labels(::Smp::Publication::ITypeRegistry* registry) {
    registry->AddArrayType(
        "Labels", // Name
        "", // Description
        ::top::inner::Uuid_Labels, // UUID
        ::top::inner::Uuid_Label, // Item Type UUID
        sizeof(::top::inner::Label), // Item Type size
        2, // Number of elements
        false // Simple array
    );
}
} // namespace inner
} // namespace top
