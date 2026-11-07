// Block notice
// line two
// -----------------------------------------------------------------------------
// File Name    : Pair.cpp
// -----------------------------------------------------------------------------
/// @file m/Pair.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <cstddef>
#include <m/Pair.h>
#include <Smp/Publication/IStructureType.h>
#include <Smp/ViewKind.h>

namespace m
{
void Pair::_Register(::Smp::Publication::ITypeRegistry* registry)
{
    auto *type = registry->AddStructureType(
        "Pair", // Name
        "", // Description
        ::m::Uuid_Pair // UUID
    );

    // Register the Fields of the Class
    type->AddField(
        "a", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        offsetof(Pair, a), // Field offset
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        false // Output
    );
    type->AddField(
        "b", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        offsetof(Pair, b), // Field offset
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        false // Output
    );
}
} // namespace m
