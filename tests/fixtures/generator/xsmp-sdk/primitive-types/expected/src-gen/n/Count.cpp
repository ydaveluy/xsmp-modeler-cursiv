// -----------------------------------------------------------------------------
// File Name    : Count.cpp
// -----------------------------------------------------------------------------
/// @file n/Count.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <limits>
#include <n/Count.h>

namespace n
{
void _Register_Count(::Smp::Publication::ITypeRegistry* registry) {
    registry->AddIntegerType(
        "Count", // Name
        "", // Description
        ::n::Uuid_Count, // UUID
        std::numeric_limits<::Smp::Int32>::min(), // Minimum
        std::numeric_limits<::Smp::Int32>::max(), // Maximum
        "kg\\m\"s", // Unit
        ::Smp::PrimitiveTypeKind::PTK_Int32 // Primitive Type Kind
    );
}
} // namespace n
