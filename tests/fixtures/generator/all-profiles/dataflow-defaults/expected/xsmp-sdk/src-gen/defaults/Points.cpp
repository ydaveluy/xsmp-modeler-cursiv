// -----------------------------------------------------------------------------
// File Name    : Points.cpp
// -----------------------------------------------------------------------------
/// @file defaults/Points.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <defaults/Points.h>
#include <Smp/Publication/IArrayType.h>

namespace defaults
{
void _Register_Points(::Smp::Publication::ITypeRegistry* registry) {
    registry->AddArrayType(
        "Points", // Name
        "", // Description
        ::defaults::Uuid_Points, // UUID
        ::defaults::Uuid_Point, // Item Type UUID
        sizeof(::defaults::Point), // Item Type size
        2, // Number of elements
        false // Simple array
    );
}
} // namespace defaults
