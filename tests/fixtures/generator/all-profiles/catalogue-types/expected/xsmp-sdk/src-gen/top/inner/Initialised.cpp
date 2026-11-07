// Copyright line one
// Copyright line two
// -----------------------------------------------------------------------------
// File Name    : Initialised.cpp
// -----------------------------------------------------------------------------
/// @file top/inner/Initialised.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <cstddef>
#include <Smp/Publication/IStructureType.h>
#include <Smp/ViewKind.h>
#include <top/inner/Initialised.h>

/// Inner namespace doc.
///  Second line.
namespace top::inner
{
void Initialised::_Register(::Smp::Publication::ITypeRegistry* registry)
{
    auto *type = registry->AddStructureType(
        "Initialised", // Name
        "", // Description
        ::top::inner::Uuid_Initialised // UUID
    );

    // Register the Fields of the Class
    type->AddField(
        "byName", // Name
        "", // Description
        ::top::inner::Uuid_Pair, // Type UUID
        offsetof(Initialised, byName), // Field offset
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        false // Output
    );
    type->AddField(
        "byPosition", // Name
        "", // Description
        ::top::inner::Uuid_Pair, // Type UUID
        offsetof(Initialised, byPosition), // Field offset
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        false // Output
    );
}
} // namespace top::inner
