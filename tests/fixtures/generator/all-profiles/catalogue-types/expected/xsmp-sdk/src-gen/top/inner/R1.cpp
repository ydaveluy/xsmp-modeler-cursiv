// Copyright line one
// Copyright line two
// -----------------------------------------------------------------------------
// File Name    : R1.cpp
// -----------------------------------------------------------------------------
/// @file top/inner/R1.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <top/inner/R1.h>

/// Inner namespace doc.
///  Second line.
namespace top::inner
{
void _Register_R1(::Smp::Publication::ITypeRegistry* registry) {
    registry->AddFloatType(
        "R1", // Name
        "", // Description
        ::top::inner::Uuid_R1, // UUID
        0.0, // Minimum
        1.0, // Maximum
        false, // Minimum exclusive
        true, // Maximum inclusive
        "", // Unit
        ::Smp::PrimitiveTypeKind::PTK_Float64 // Primitive Type Kind
    );
}
} // namespace top::inner
