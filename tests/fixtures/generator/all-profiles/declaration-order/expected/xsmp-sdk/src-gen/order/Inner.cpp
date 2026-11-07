// -----------------------------------------------------------------------------
// File Name    : Inner.cpp
// -----------------------------------------------------------------------------
/// @file order/Inner.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <order/Inner.h>
#include <Smp/Publication/IArrayType.h>

namespace order
{
void _Register_Inner(::Smp::Publication::ITypeRegistry* registry) {
    registry->AddArrayType(
        "Inner", // Name
        "", // Description
        ::order::Uuid_Inner, // UUID
        ::order::Uuid_Value, // Item Type UUID
        sizeof(::order::Value), // Item Type size
        2, // Number of elements
        false // Simple array
    );
}
} // namespace order
