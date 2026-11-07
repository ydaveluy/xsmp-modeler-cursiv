// Copyright line one
// Copyright line two
// -----------------------------------------------------------------------------
// File Name    : R2.cpp
// -----------------------------------------------------------------------------
/// @file top/inner/R2.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <top/inner/R2.h>

/// Top namespace doc.
namespace top
{
/// Inner namespace doc.
///  Second line.
namespace inner
{
void _Register_R2(::Smp::Publication::ITypeRegistry* registry) {
    registry->AddFloatType(
        "R2", // Name
        "", // Description
        ::top::inner::Uuid_R2, // UUID
        0.0, // Minimum
        1.0, // Maximum
        true, // Minimum inclusive
        false, // Maximum exclusive
        "", // Unit
        ::Smp::PrimitiveTypeKind::PTK_Float64 // Primitive Type Kind
    );
}
} // namespace inner
} // namespace top
