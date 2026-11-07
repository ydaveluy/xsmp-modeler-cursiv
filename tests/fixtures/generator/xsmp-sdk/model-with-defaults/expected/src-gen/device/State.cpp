// -----------------------------------------------------------------------------
// File Name    : State.cpp
// -----------------------------------------------------------------------------
/// @file device/State.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <cstddef>
#include <device/State.h>
#include <Smp/Publication/IStructureType.h>
#include <Smp/ViewKind.h>

namespace device
{
void State::_Register(::Smp::Publication::ITypeRegistry* registry)
{
    auto *type = registry->AddStructureType(
        "State", // Name
        "", // Description
        ::device::Uuid_State // UUID
    );

    // Register the Fields of the Class
    type->AddField(
        "position", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Float64, // Type UUID
        offsetof(State, position), // Field offset
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        false // Output
    );
    type->AddField(
        "mode", // Name
        "", // Description
        ::device::Uuid_Mode, // Type UUID
        offsetof(State, mode), // Field offset
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        false // Output
    );
}
} // namespace device
