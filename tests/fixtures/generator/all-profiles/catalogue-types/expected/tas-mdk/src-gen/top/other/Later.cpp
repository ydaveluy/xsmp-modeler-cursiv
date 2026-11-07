// Copyright line one
// Copyright line two
// -----------------------------------------------------------------------------
// File Name    : Later.cpp
// -----------------------------------------------------------------------------
/// @file top/other/Later.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <Smp/Publication/IArrayType.h>
#include <top/other/Later.h>

/// Top namespace doc.
namespace top
{
namespace other
{
void _Register_Later(::Smp::Publication::ITypeRegistry* registry) {
    registry->AddArrayType(
        "Later", // Name
        "", // Description
        ::top::other::Uuid_Later, // UUID
        ::top::other::Uuid_Earlier, // Item Type UUID
        sizeof(::top::other::Earlier), // Item Type size
        2, // Number of elements
        false // Simple array
    );
}
} // namespace other
} // namespace top
