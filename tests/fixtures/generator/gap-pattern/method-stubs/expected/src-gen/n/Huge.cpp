// -----------------------------------------------------------------------------
// File Name    : Huge.cpp
// -----------------------------------------------------------------------------
/// @file n/Huge.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <n/Huge.h>
#include <Smp/Publication/IArrayType.h>

namespace n
{
void _Register_Huge(::Smp::Publication::ITypeRegistry* registry) {
    registry->AddArrayType(
        "Huge", // Name
        "", // Description
        ::n::Uuid_Huge, // UUID
        ::Smp::Uuids::Uuid_Int32, // Item Type UUID
        sizeof(::Smp::Int32), // Item Type size
        10000000, // Number of elements
        false // Simple array
    );
}
} // namespace n
