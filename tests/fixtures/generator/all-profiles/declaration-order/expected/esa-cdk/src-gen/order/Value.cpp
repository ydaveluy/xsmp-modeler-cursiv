// -----------------------------------------------------------------------------
// File Name    : Value.cpp
// -----------------------------------------------------------------------------
/// @file order/Value.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <cstddef>
#include <order/Value.h>
#include <Smp/Publication/IStructureType.h>
#include <Smp/ViewKind.h>

namespace order
{
void Value::_Register(::Smp::Publication::ITypeRegistry* registry)
{
    auto *type = registry->AddStructureType(
        "Value", // Name
        "", // Description
        ::order::Uuid_Value // UUID
    );

    // Register the Fields of the Class
    type->AddField(
        "value", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        offsetof(Value, value), // Field offset
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        false // Output
    );
}
} // namespace order
