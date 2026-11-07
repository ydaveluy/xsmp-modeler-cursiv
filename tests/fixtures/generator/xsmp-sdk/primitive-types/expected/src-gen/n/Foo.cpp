// -----------------------------------------------------------------------------
// File Name    : Foo.cpp
// -----------------------------------------------------------------------------
/// @file n/Foo.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <limits>
#include <n/Foo.h>

namespace n
{
void _Register_Foo(::Smp::Publication::ITypeRegistry* registry) {
    registry->AddIntegerType(
        "Foo", // Name
        "", // Description
        ::n::Uuid_Foo, // UUID
        std::numeric_limits<::Smp::Int32>::min(), // Minimum
        std::numeric_limits<::Smp::Int32>::max(), // Maximum
        "", // Unit
        ::Smp::PrimitiveTypeKind::PTK_Int32 // Primitive Type Kind
    );
}
} // namespace n
