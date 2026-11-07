// Copyright line one
// Copyright line two
// -----------------------------------------------------------------------------
// File Name    : NotSimple.cpp
// -----------------------------------------------------------------------------
/// @file top/inner/NotSimple.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <Smp/Publication/IArrayType.h>
#include <top/inner/NotSimple.h>

/// Inner namespace doc.
///  Second line.
namespace top::inner
{
void _Register_NotSimple(::Smp::Publication::ITypeRegistry* registry) {
    registry->AddArrayType(
        "NotSimple", // Name
        "", // Description
        ::top::inner::Uuid_NotSimple, // UUID
        ::Smp::Uuids::Uuid_Float64, // Item Type UUID
        sizeof(::Smp::Float64), // Item Type size
        2, // Number of elements
        false // Simple array
    );
}
} // namespace top::inner
