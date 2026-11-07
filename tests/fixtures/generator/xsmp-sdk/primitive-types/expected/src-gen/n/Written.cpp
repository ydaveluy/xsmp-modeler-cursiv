// -----------------------------------------------------------------------------
// File Name    : Written.cpp
// -----------------------------------------------------------------------------
/// @file n/Written.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <n/Written.h>

namespace n
{
void _Register_Written(::Smp::Publication::ITypeRegistry* registry) {
    registry->AddIntegerType(
        "Written", // Name
        "", // Description
        ::n::Uuid_Written, // UUID
        0, // Minimum
        9223372036854775807LL, // Maximum
        "", // Unit
        ::Smp::PrimitiveTypeKind::PTK_UInt64 // Primitive Type Kind
    );
}
} // namespace n
