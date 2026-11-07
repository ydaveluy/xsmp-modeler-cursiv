// -----------------------------------------------------------------------------
// File Name    : Payload.cpp
// -----------------------------------------------------------------------------
/// @file n/Payload.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <cstddef>
#include <n/Payload.h>
#include <Smp/Publication/IStructureType.h>
#include <Smp/ViewKind.h>

namespace n
{
void Payload::_Register(::Smp::Publication::ITypeRegistry* registry)
{
    auto *type = registry->AddStructureType(
        "Payload", // Name
        "", // Description
        ::n::Uuid_Payload // UUID
    );

    // Register the Fields of the Class
    type->AddField(
        "enabled", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Bool, // Type UUID
        offsetof(Payload, enabled), // Field offset
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        false // Output
    );
}
} // namespace n
