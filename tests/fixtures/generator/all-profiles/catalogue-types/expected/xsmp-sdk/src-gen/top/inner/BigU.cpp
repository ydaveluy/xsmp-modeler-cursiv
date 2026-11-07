// Copyright line one
// Copyright line two
// -----------------------------------------------------------------------------
// File Name    : BigU.cpp
// -----------------------------------------------------------------------------
/// @file top/inner/BigU.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <top/inner/BigU.h>

/// Inner namespace doc.
///  Second line.
namespace top::inner
{
void _Register_BigU(::Smp::Publication::ITypeRegistry* registry) {
    registry->AddIntegerType(
        "BigU", // Name
        "", // Description
        ::top::inner::Uuid_BigU, // UUID
        0, // Minimum
        9223372036854775807LL, // Maximum
        "", // Unit
        ::Smp::PrimitiveTypeKind::PTK_UInt64 // Primitive Type Kind
    );
}
} // namespace top::inner
