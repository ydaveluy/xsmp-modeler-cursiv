// Copyright line one
// Copyright line two
// -----------------------------------------------------------------------------
// File Name    : Matrix.cpp
// -----------------------------------------------------------------------------
/// @file top/inner/Matrix.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <Smp/Publication/IArrayType.h>
#include <top/inner/Matrix.h>

/// Inner namespace doc.
///  Second line.
namespace top::inner
{
void _Register_Matrix(::Smp::Publication::ITypeRegistry* registry) {
    registry->AddArrayType(
        "Matrix", // Name
        "", // Description
        ::top::inner::Uuid_Matrix, // UUID
        ::top::inner::Uuid_Ints, // Item Type UUID
        sizeof(::top::inner::Ints), // Item Type size
        2, // Number of elements
        false // Simple array
    );
}
} // namespace top::inner
