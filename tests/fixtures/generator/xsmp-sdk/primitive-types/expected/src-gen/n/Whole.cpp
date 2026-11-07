// -----------------------------------------------------------------------------
// File Name    : Whole.cpp
// -----------------------------------------------------------------------------
/// @file n/Whole.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <limits>
#include <n/Whole.h>

namespace n
{
void _Register_Whole(::Smp::Publication::ITypeRegistry* registry) {
    registry->AddIntegerType(
        "Whole", // Name
        "", // Description
        ::n::Uuid_Whole, // UUID
        std::numeric_limits<::Smp::UInt64>::min(), // Minimum
        9223372036854775807LL, // Maximum
        "", // Unit
        ::Smp::PrimitiveTypeKind::PTK_UInt64 // Primitive Type Kind
    );
}
} // namespace n
