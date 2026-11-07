// -----------------------------------------------------------------------------
// File Name    : Pair.cpp
// -----------------------------------------------------------------------------
/// @file n/Pair.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <n/Pair.h>
#include <Smp/Publication/IArrayType.h>

namespace n
{
void _Register_Pair(::Smp::Publication::ITypeRegistry* registry) {
    registry->AddArrayType(
        "Pair", // Name
        "", // Description
        ::n::Uuid_Pair, // UUID
        ::n::Uuid_Single, // Item Type UUID
        sizeof(::n::Single), // Item Type size
        2, // Number of elements
        false // Simple array
    );
}
} // namespace n
