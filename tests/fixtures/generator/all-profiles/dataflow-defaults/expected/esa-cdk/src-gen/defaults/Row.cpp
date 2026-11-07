// -----------------------------------------------------------------------------
// File Name    : Row.cpp
// -----------------------------------------------------------------------------
/// @file defaults/Row.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <defaults/Row.h>
#include <Smp/Publication/IArrayType.h>

namespace defaults
{
void _Register_Row(::Smp::Publication::ITypeRegistry* registry) {
    registry->AddArrayType(
        "Row", // Name
        "", // Description
        ::defaults::Uuid_Row, // UUID
        ::Smp::Uuids::Uuid_Int32, // Item Type UUID
        sizeof(::Smp::Int32), // Item Type size
        2, // Number of elements
        false // Simple array
    );
}
} // namespace defaults
