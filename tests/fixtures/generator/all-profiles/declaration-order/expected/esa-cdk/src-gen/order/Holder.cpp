// -----------------------------------------------------------------------------
// File Name    : Holder.cpp
// -----------------------------------------------------------------------------
/// @file order/Holder.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <cstddef>
#include <order/Holder.h>
#include <Smp/Publication/IStructureType.h>
#include <Smp/ViewKind.h>

namespace order
{
void Holder::_Register(::Smp::Publication::ITypeRegistry* registry)
{
    auto *type = registry->AddStructureType(
        "Holder", // Name
        "", // Description
        ::order::Uuid_Holder // UUID
    );

    // Register the Fields of the Class
    type->AddField(
        "first", // Name
        "", // Description
        ::order::Uuid_Value, // Type UUID
        offsetof(Holder, first), // Field offset
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        false // Output
    );
    type->AddField(
        "second", // Name
        "", // Description
        ::order::Uuid_Value, // Type UUID
        offsetof(Holder, second), // Field offset
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        false // Output
    );
}
} // namespace order
