// -----------------------------------------------------------------------------
// File Name    : Ratio.cpp
// -----------------------------------------------------------------------------
/// @file n/Ratio.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <n/Ratio.h>

namespace n
{
void _Register_Ratio(::Smp::Publication::ITypeRegistry* registry) {
    registry->AddFloatType(
        "Ratio", // Name
        "", // Description
        ::n::Uuid_Ratio, // UUID
        0.0, // Minimum
        1.0, // Maximum
        false, // Minimum exclusive
        true, // Maximum inclusive
        "", // Unit
        ::Smp::PrimitiveTypeKind::PTK_Float64 // Primitive Type Kind
    );
}
} // namespace n
