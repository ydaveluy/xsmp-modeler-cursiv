// -----------------------------------------------------------------------------
// File Name    : Hypercube.cpp
// -----------------------------------------------------------------------------
/// @file defaults/Hypercube.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <defaults/Hypercube.h>
#include <Smp/Publication/IArrayType.h>

namespace defaults
{
void _Register_Hypercube(::Smp::Publication::ITypeRegistry* registry) {
    registry->AddArrayType(
        "Hypercube", // Name
        "", // Description
        ::defaults::Uuid_Hypercube, // UUID
        ::defaults::Uuid_Cube, // Item Type UUID
        sizeof(::defaults::Cube), // Item Type size
        2, // Number of elements
        false // Simple array
    );
}
} // namespace defaults
