// -----------------------------------------------------------------------------
// File Name    : P.cpp
// -----------------------------------------------------------------------------
/// @file n/P.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <limits>
#include <n/P.h>

namespace n
{
void _Register_P(::Smp::Publication::ITypeRegistry* registry) {
    registry->AddIntegerType(
        "P", // Name
        "", // Description
        ::n::Uuid_P, // UUID
        std::numeric_limits<::Smp::Int32>::min(), // Minimum
        std::numeric_limits<::Smp::Int32>::max(), // Maximum
        "", // Unit
        ::Smp::PrimitiveTypeKind::PTK_Int32 // Primitive Type Kind
    );
}
} // namespace n
