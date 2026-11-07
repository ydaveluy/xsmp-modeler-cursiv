// -----------------------------------------------------------------------------
// File Name    : Grid.cpp
// -----------------------------------------------------------------------------
/// @file n/Grid.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <n/Grid.h>
#include <Smp/Publication/IArrayType.h>

namespace n
{
void _Register_Grid(::Smp::Publication::ITypeRegistry* registry) {
    registry->AddArrayType(
        "Grid", // Name
        "", // Description
        ::n::Uuid_Grid, // UUID
        ::n::Uuid_Row, // Item Type UUID
        sizeof(::n::Row), // Item Type size
        2, // Number of elements
        false // Simple array
    );
}
} // namespace n
