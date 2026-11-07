// -----------------------------------------------------------------------------
// File Name    : Hostile.cpp
// -----------------------------------------------------------------------------
/// @file n/Hostile.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <limits>
#include <n/Hostile.h>

namespace n
{
void _Register_Hostile(::Smp::Publication::ITypeRegistry* registry) {
    registry->AddFloatType(
        "Hostile", // Name
        "", // Description
        ::n::Uuid_Hostile, // UUID
        std::numeric_limits<::Smp::Float64>::lowest(), // Minimum
        std::numeric_limits<::Smp::Float64>::max(), // Maximum
        true, // Minimum inclusive
        true, // Maximum inclusive
        "kg\\m\"s", // Unit
        ::Smp::PrimitiveTypeKind::PTK_Float64 // Primitive Type Kind
    );
}
} // namespace n
