// -----------------------------------------------------------------------------
// File Name    : Packets.cpp
// -----------------------------------------------------------------------------
/// @file defaults/Packets.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <defaults/Packets.h>
#include <Smp/Publication/IArrayType.h>

namespace defaults
{
void _Register_Packets(::Smp::Publication::ITypeRegistry* registry) {
    registry->AddArrayType(
        "Packets", // Name
        "", // Description
        ::defaults::Uuid_Packets, // UUID
        ::defaults::Uuid_Packet, // Item Type UUID
        sizeof(::defaults::Packet), // Item Type size
        2, // Number of elements
        false // Simple array
    );
}
} // namespace defaults
