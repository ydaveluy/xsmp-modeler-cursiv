// Copyright line one
// Copyright line two
// -----------------------------------------------------------------------------
// File Name    : Earlier.cpp
// -----------------------------------------------------------------------------
/// @file top/other/Earlier.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <cstddef>
#include <Smp/Publication/IStructureType.h>
#include <Smp/ViewKind.h>
#include <top/other/Earlier.h>

namespace top::other
{
void Earlier::_Register(::Smp::Publication::ITypeRegistry* registry)
{
    auto *type = registry->AddStructureType(
        "Earlier", // Name
        "", // Description
        ::top::other::Uuid_Earlier // UUID
    );

    // Register the Fields of the Class
    type->AddField(
        "x", // Name
        "", // Description
        ::top::other::Uuid_Later2, // Type UUID
        offsetof(Earlier, x), // Field offset
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        false // Output
    );
}
} // namespace top::other
