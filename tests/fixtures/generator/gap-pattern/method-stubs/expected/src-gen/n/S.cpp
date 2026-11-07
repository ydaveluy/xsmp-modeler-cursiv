// -----------------------------------------------------------------------------
// File Name    : S.cpp
// -----------------------------------------------------------------------------
/// @file n/S.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <cstddef>
#include <n/S.h>
#include <Smp/Publication/IStructureType.h>
#include <Smp/ViewKind.h>

namespace n
{
void S::_Register(::Smp::Publication::ITypeRegistry* registry)
{
    auto *type = registry->AddStructureType(
        "S", // Name
        "", // Description
        ::n::Uuid_S // UUID
    );

    // Register the Fields of the Class
    type->AddField(
        "items", // Name
        "", // Description
        ::n::Uuid_Huge, // Type UUID
        offsetof(S, items), // Field offset
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        false // Output
    );
}
} // namespace n
