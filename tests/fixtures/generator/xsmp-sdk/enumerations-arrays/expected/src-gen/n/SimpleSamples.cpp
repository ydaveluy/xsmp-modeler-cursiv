// -----------------------------------------------------------------------------
// File Name    : SimpleSamples.cpp
// -----------------------------------------------------------------------------
/// @file n/SimpleSamples.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <n/SimpleSamples.h>
#include <Smp/Publication/IArrayType.h>

namespace n
{
void _Register_SimpleSamples(::Smp::Publication::ITypeRegistry* registry) {
    registry->AddArrayType(
        "SimpleSamples", // Name
        "", // Description
        ::n::Uuid_SimpleSamples, // UUID
        ::Smp::Uuids::Uuid_Float32, // Item Type UUID
        sizeof(::Smp::Float32), // Item Type size
        16, // Number of elements
        true // Simple array
    );
}
} // namespace n
