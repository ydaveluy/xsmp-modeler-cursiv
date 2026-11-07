// -----------------------------------------------------------------------------
// File Name    : Narrow.cpp
// -----------------------------------------------------------------------------
/// @file n/Narrow.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <limits>
#include <n/Narrow.h>

namespace n
{
void _Register_Narrow(::Smp::Publication::ITypeRegistry* registry) {
    registry->AddIntegerType(
        "Narrow", // Name
        "", // Description
        ::n::Uuid_Narrow, // UUID
        std::numeric_limits<::Smp::UInt32>::min(), // Minimum
        std::numeric_limits<::Smp::UInt32>::max(), // Maximum
        "", // Unit
        ::Smp::PrimitiveTypeKind::PTK_UInt32 // Primitive Type Kind
    );
}
} // namespace n
