// -----------------------------------------------------------------------------
// File Name    : Cube.cpp
// -----------------------------------------------------------------------------
/// @file defaults/Cube.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <defaults/Cube.h>
#include <Smp/Publication/IArrayType.h>

namespace defaults
{
void _Register_Cube(::Smp::Publication::ITypeRegistry* registry) {
    registry->AddArrayType(
        "Cube", // Name
        "", // Description
        ::defaults::Uuid_Cube, // UUID
        ::defaults::Uuid_Matrix, // Item Type UUID
        sizeof(::defaults::Matrix), // Item Type size
        2, // Number of elements
        false // Simple array
    );
}
} // namespace defaults
