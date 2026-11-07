// -----------------------------------------------------------------------------
// File Name    : Packet.cpp
// -----------------------------------------------------------------------------
/// @file defaults/Packet.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <cstddef>
#include <defaults/Packet.h>
#include <Smp/Publication/IStructureType.h>
#include <Smp/ViewKind.h>

namespace defaults
{
void Packet::_Register(::Smp::Publication::ITypeRegistry* registry)
{
    auto *type = registry->AddStructureType(
        "Packet", // Name
        "", // Description
        ::defaults::Uuid_Packet // UUID
    );

    // Register the Fields of the Class
    type->AddField(
        "samples", // Name
        "", // Description
        ::defaults::Uuid_Row, // Type UUID
        offsetof(Packet, samples), // Field offset
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        false // Output
    );
    type->AddField(
        "tag", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        offsetof(Packet, tag), // Field offset
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        false // Output
    );
}
} // namespace defaults
