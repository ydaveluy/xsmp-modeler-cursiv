// -----------------------------------------------------------------------------
// File Name    : Any.cpp
// -----------------------------------------------------------------------------
/// @file n/Any.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <limits>
#include <n/Any.h>

namespace n
{
void _Register_Any(::Smp::Publication::ITypeRegistry* registry) {
    registry->AddIntegerType(
        "Any", // Name
        "", // Description
        ::n::Uuid_Any, // UUID
        std::numeric_limits<::Smp::Int32>::min(), // Minimum
        std::numeric_limits<::Smp::Int32>::max(), // Maximum
        "", // Unit
        ::Smp::PrimitiveTypeKind::PTK_Int32 // Primitive Type Kind
    );
}
} // namespace n
