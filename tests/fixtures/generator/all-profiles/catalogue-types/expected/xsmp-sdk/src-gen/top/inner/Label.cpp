// Copyright line one
// Copyright line two
// -----------------------------------------------------------------------------
// File Name    : Label.cpp
// -----------------------------------------------------------------------------
/// @file top/inner/Label.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <Smp/Publication/IType.h>
#include <top/inner/Label.h>

/// Inner namespace doc.
///  Second line.
namespace top::inner
{
void _Register_Label(::Smp::Publication::ITypeRegistry* registry) {
    registry->AddStringType(
        "Label", // Name
        "A label.", // Description
        ::top::inner::Uuid_Label, // UUID
        1'024 // Length
    );
}
} // namespace top::inner
