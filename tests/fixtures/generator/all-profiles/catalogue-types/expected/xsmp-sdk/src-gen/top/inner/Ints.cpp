// Copyright line one
// Copyright line two
// -----------------------------------------------------------------------------
// File Name    : Ints.cpp
// -----------------------------------------------------------------------------
/// @file top/inner/Ints.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <Smp/Publication/IArrayType.h>
#include <top/inner/Ints.h>

/// Inner namespace doc.
///  Second line.
namespace top::inner
{
void _Register_Ints(::Smp::Publication::ITypeRegistry* registry) {
    registry->AddArrayType(
        "Ints", // Name
        "", // Description
        ::top::inner::Uuid_Ints, // UUID
        ::Smp::Uuids::Uuid_Int32, // Item Type UUID
        sizeof(::Smp::Int32), // Item Type size
        3, // Number of elements
        false // Simple array
    );
}
} // namespace top::inner
