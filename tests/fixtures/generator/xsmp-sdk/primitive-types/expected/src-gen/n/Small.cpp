// -----------------------------------------------------------------------------
// File Name    : Small.cpp
// -----------------------------------------------------------------------------
/// @file n/Small.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <n/Small.h>

namespace n
{
void _Register_Small(::Smp::Publication::ITypeRegistry* registry) {
    registry->AddIntegerType(
        "Small", // Name
        "", // Description
        ::n::Uuid_Small, // UUID
        0, // Minimum
        100, // Maximum
        "", // Unit
        ::Smp::PrimitiveTypeKind::PTK_UInt64 // Primitive Type Kind
    );
}
} // namespace n
