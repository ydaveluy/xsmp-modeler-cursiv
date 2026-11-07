// -----------------------------------------------------------------------------
// File Name    : Pct.cpp
// -----------------------------------------------------------------------------
/// @file n/Pct.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <n/Pct.h>

namespace n
{
void _Register_Pct(::Smp::Publication::ITypeRegistry* registry) {
    registry->AddIntegerType(
        "Pct", // Name
        "A percentage.", // Description
        ::n::Uuid_Pct, // UUID
        0, // Minimum
        100, // Maximum
        "%", // Unit
        ::Smp::PrimitiveTypeKind::PTK_Int32 // Primitive Type Kind
    );
}
} // namespace n
