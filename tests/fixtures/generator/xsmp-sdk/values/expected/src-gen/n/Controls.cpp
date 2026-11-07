// -----------------------------------------------------------------------------
// File Name    : Controls.cpp
// -----------------------------------------------------------------------------
/// @file n/Controls.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <cstddef>
#include <n/Controls.h>
#include <Smp/Publication/IStructureType.h>
#include <Smp/ViewKind.h>

namespace n
{
void Controls::_Register(::Smp::Publication::ITypeRegistry* registry)
{
    auto *type = registry->AddStructureType(
        "Controls", // Name
        "", // Description
        ::n::Uuid_Controls // UUID
    );

    // Register the Fields of the Class
    type->AddField(
        "field1", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Char8, // Type UUID
        offsetof(Controls, field1), // Field offset
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        false // Output
    );
    type->AddField(
        "field2", // Name
        "", // Description
        ::n::Uuid_Name, // Type UUID
        offsetof(Controls, field2), // Field offset
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        false // Output
    );
}
} // namespace n
