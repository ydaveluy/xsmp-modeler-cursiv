// Copyright line one
// Copyright line two
// -----------------------------------------------------------------------------
// File Name    : SmallU.cpp
// -----------------------------------------------------------------------------
/// @file top/inner/SmallU.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <top/inner/SmallU.h>

/// Inner namespace doc.
///  Second line.
namespace top::inner
{
void _Register_SmallU(::Smp::Publication::ITypeRegistry* registry) {
    registry->AddIntegerType(
        "SmallU", // Name
        "", // Description
        ::top::inner::Uuid_SmallU, // UUID
        0, // Minimum
        1000, // Maximum
        "", // Unit
        ::Smp::PrimitiveTypeKind::PTK_UInt64 // Primitive Type Kind
    );
}
} // namespace top::inner
