// -----------------------------------------------------------------------------
// File Name    : Items.cpp
// -----------------------------------------------------------------------------
/// @file n/Items.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <n/Items.h>
#include <Smp/Publication/IArrayType.h>

namespace n
{
void _Register_Items(::Smp::Publication::ITypeRegistry* registry) {
    registry->AddArrayType(
        "Items", // Name
        "", // Description
        ::n::Uuid_Items, // UUID
        ::n::Uuid_WithOutput, // Item Type UUID
        sizeof(::n::WithOutput), // Item Type size
        2, // Number of elements
        false // Simple array
    );
}
} // namespace n
