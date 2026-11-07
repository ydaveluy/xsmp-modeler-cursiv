// Copyright line one
// Copyright line two
// -----------------------------------------------------------------------------
// File Name    : Plain.cpp
// -----------------------------------------------------------------------------
/// @file top/inner/Plain.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <limits>
#include <top/inner/Plain.h>

/// Top namespace doc.
namespace top
{
/// Inner namespace doc.
///  Second line.
namespace inner
{
void _Register_Plain(::Smp::Publication::ITypeRegistry* registry) {
    registry->AddIntegerType(
        "Plain", // Name
        "", // Description
        ::top::inner::Uuid_Plain, // UUID
        std::numeric_limits<::Smp::Int32>::min(), // Minimum
        std::numeric_limits<::Smp::Int32>::max(), // Maximum
        "", // Unit
        ::Smp::PrimitiveTypeKind::PTK_Int32 // Primitive Type Kind
    );
}
} // namespace inner
} // namespace top
