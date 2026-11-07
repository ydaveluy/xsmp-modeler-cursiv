// -----------------------------------------------------------------------------
// File Name    : Row.cpp
// -----------------------------------------------------------------------------
/// @file n/Row.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <n/Row.h>
#include <Smp/Publication/IArrayType.h>

namespace n
{
void _Register_Row(::Smp::Publication::ITypeRegistry* registry) {
    registry->AddArrayType(
        "Row", // Name
        "", // Description
        ::n::Uuid_Row, // UUID
        ::Smp::Uuids::Uuid_Int32, // Item Type UUID
        sizeof(::Smp::Int32), // Item Type size
        2, // Number of elements
        false // Simple array
    );
}
} // namespace n
