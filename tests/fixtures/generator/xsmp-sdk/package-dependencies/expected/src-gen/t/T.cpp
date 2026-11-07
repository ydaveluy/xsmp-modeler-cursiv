// -----------------------------------------------------------------------------
// File Name    : T.cpp
// -----------------------------------------------------------------------------
/// @file t/T.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <limits>
#include <t/T.h>

namespace t
{
void _Register_T(::Smp::Publication::ITypeRegistry* registry) {
    registry->AddIntegerType(
        "T", // Name
        "", // Description
        ::t::Uuid_T, // UUID
        std::numeric_limits<::Smp::Int32>::min(), // Minimum
        std::numeric_limits<::Smp::Int32>::max(), // Maximum
        "", // Unit
        ::Smp::PrimitiveTypeKind::PTK_Int32 // Primitive Type Kind
    );
}
} // namespace t
