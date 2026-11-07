// -----------------------------------------------------------------------------
// File Name    : Outer.cpp
// -----------------------------------------------------------------------------
/// @file order/Outer.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <order/Outer.h>
#include <Smp/Publication/IArrayType.h>

namespace order
{
void _Register_Outer(::Smp::Publication::ITypeRegistry* registry) {
    registry->AddArrayType(
        "Outer", // Name
        "", // Description
        ::order::Uuid_Outer, // UUID
        ::order::Uuid_Inner, // Item Type UUID
        sizeof(::order::Inner), // Item Type size
        2, // Number of elements
        false // Simple array
    );
}
} // namespace order
