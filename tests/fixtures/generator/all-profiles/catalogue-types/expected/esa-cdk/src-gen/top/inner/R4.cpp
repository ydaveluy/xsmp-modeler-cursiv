// Copyright line one
// Copyright line two
// -----------------------------------------------------------------------------
// File Name    : R4.cpp
// -----------------------------------------------------------------------------
/// @file top/inner/R4.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <limits>
#include <top/inner/R4.h>

/// Top namespace doc.
namespace top
{
/// Inner namespace doc.
///  Second line.
namespace inner
{
void _Register_R4(::Smp::Publication::ITypeRegistry* registry) {
    registry->AddFloatType(
        "R4", // Name
        "", // Description
        ::top::inner::Uuid_R4, // UUID
        std::numeric_limits<::Smp::Float64>::lowest(), // Minimum
        std::numeric_limits<::Smp::Float64>::max(), // Maximum
        true, // Minimum inclusive
        true, // Maximum inclusive
        "", // Unit
        ::Smp::PrimitiveTypeKind::PTK_Float64 // Primitive Type Kind
    );
}
} // namespace inner
} // namespace top
