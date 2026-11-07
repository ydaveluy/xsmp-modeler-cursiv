// Block notice
// line two
// -----------------------------------------------------------------------------
// File Name    : Three.cpp
// -----------------------------------------------------------------------------
/// @file m/Three.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <m/Three.h>
#include <Smp/Publication/IArrayType.h>

namespace m
{
void _Register_Three(::Smp::Publication::ITypeRegistry* registry) {
    registry->AddArrayType(
        "Three", // Name
        "", // Description
        ::m::Uuid_Three, // UUID
        ::Smp::Uuids::Uuid_Int32, // Item Type UUID
        sizeof(::Smp::Int32), // Item Type size
        3, // Number of elements
        false // Simple array
    );
}
} // namespace m
