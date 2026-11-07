// -----------------------------------------------------------------------------
// File Name    : Matrix.cpp
// -----------------------------------------------------------------------------
/// @file defaults/Matrix.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <defaults/Matrix.h>
#include <Smp/Publication/IArrayType.h>

namespace defaults
{
void _Register_Matrix(::Smp::Publication::ITypeRegistry* registry) {
    registry->AddArrayType(
        "Matrix", // Name
        "", // Description
        ::defaults::Uuid_Matrix, // UUID
        ::defaults::Uuid_Row, // Item Type UUID
        sizeof(::defaults::Row), // Item Type size
        2, // Number of elements
        false // Simple array
    );
}
} // namespace defaults
