// Copyright line one
// Copyright line two
// -----------------------------------------------------------------------------
// File Name    : Later2.cpp
// -----------------------------------------------------------------------------
/// @file top/other/Later2.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <Smp/Publication/IArrayType.h>
#include <top/other/Later2.h>

/// Top namespace doc.
namespace top
{
namespace other
{
void _Register_Later2(::Smp::Publication::ITypeRegistry* registry) {
    registry->AddArrayType(
        "Later2", // Name
        "", // Description
        ::top::other::Uuid_Later2, // UUID
        ::Smp::Uuids::Uuid_Int32, // Item Type UUID
        sizeof(::Smp::Int32), // Item Type size
        1, // Number of elements
        false // Simple array
    );
}
} // namespace other
} // namespace top
