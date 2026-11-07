// -----------------------------------------------------------------------------
// File Name    : State.cpp
// -----------------------------------------------------------------------------
/// @file n/State.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <cstddef>
#include <n/State.h>
#include <Smp/Publication/IStructureType.h>
#include <Smp/ViewKind.h>

namespace n
{
void State::_Register(::Smp::Publication::ITypeRegistry* registry)
{
    auto *type = registry->AddStructureType(
        "State", // Name
        "", // Description
        ::n::Uuid_State // UUID
    );

    // Register the Fields of the Class
    type->AddField(
        "ratio", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Float64, // Type UUID
        offsetof(State, ratio), // Field offset
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        false // Output
    );
    type->AddField(
        "cycle", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Duration, // Type UUID
        offsetof(State, cycle), // Field offset
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        false // Output
    );
    type->AddField(
        "epoch", // Name
        "", // Description
        ::Smp::Uuids::Uuid_DateTime, // Type UUID
        offsetof(State, epoch), // Field offset
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        false // Output
    );
}
} // namespace n
