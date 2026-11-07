// -----------------------------------------------------------------------------
// File Name    : Samples.cpp
// -----------------------------------------------------------------------------
/// @file n/Samples.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <n/Samples.h>
#include <Smp/Publication/IArrayType.h>

namespace n
{
void _Register_Samples(::Smp::Publication::ITypeRegistry* registry) {
    registry->AddArrayType(
        "Samples", // Name
        "", // Description
        ::n::Uuid_Samples, // UUID
        ::Smp::Uuids::Uuid_Float32, // Item Type UUID
        sizeof(::Smp::Float32), // Item Type size
        16, // Number of elements
        false // Simple array
    );
}
} // namespace n
