// -----------------------------------------------------------------------------
// File Name    : Excluded.cpp
// -----------------------------------------------------------------------------
/// @file n/Excluded.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <cstddef>
#include <n/Excluded.h>
#include <Smp/Publication/IStructureType.h>
#include <Smp/ViewKind.h>

namespace n
{
// Field shared
::Smp::Int32 Excluded::shared{};

void Excluded::_Register(::Smp::Publication::ITypeRegistry* registry)
{
    auto *type = registry->AddStructureType(
        "Excluded", // Name
        "", // Description
        ::n::Uuid_Excluded // UUID
    );

    // Register the Fields of the Class
    type->AddField(
        "kept", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        offsetof(Excluded, kept), // Field offset
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        false // Output
    );
}
} // namespace n
