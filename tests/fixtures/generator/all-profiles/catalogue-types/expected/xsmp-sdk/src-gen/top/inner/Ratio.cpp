// Copyright line one
// Copyright line two
// -----------------------------------------------------------------------------
// File Name    : Ratio.cpp
// -----------------------------------------------------------------------------
/// @file top/inner/Ratio.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <top/inner/Ratio.h>

/// Inner namespace doc.
///  Second line.
namespace top::inner
{
void _Register_Ratio(::Smp::Publication::ITypeRegistry* registry) {
    registry->AddFloatType(
        "Ratio", // Name
        "", // Description
        ::top::inner::Uuid_Ratio, // UUID
        0.0, // Minimum
        1.0, // Maximum
        true, // Minimum inclusive
        true, // Maximum inclusive
        "\"quoted\"", // Unit
        ::Smp::PrimitiveTypeKind::PTK_Float32 // Primitive Type Kind
    );
}
} // namespace top::inner
