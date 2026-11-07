// Copyright line one
// Copyright line two
// -----------------------------------------------------------------------------
// File Name    : Speed.cpp
// -----------------------------------------------------------------------------
/// @file top/inner/Speed.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <top/inner/Speed.h>

/// Top namespace doc.
namespace top
{
/// Inner namespace doc.
///  Second line.
namespace inner
{
void _Register_Speed(::Smp::Publication::ITypeRegistry* registry) {
    registry->AddIntegerType(
        "Speed", // Name
        "", // Description
        ::top::inner::Uuid_Speed, // UUID
        0, // Minimum
        1000, // Maximum
        "m/s", // Unit
        ::Smp::PrimitiveTypeKind::PTK_Int16 // Primitive Type Kind
    );
}
} // namespace inner
} // namespace top
