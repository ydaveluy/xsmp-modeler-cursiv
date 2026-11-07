// Copyright line one
// Copyright line two
// -----------------------------------------------------------------------------
// File Name    : Nested.cpp
// -----------------------------------------------------------------------------
/// @file top/inner/Nested.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <cstddef>
#include <Smp/Publication/IStructureType.h>
#include <Smp/ViewKind.h>
#include <top/inner/Nested.h>

/// Inner namespace doc.
///  Second line.
namespace top::inner
{
void Nested::_Register(::Smp::Publication::ITypeRegistry* registry)
{
    auto *type = registry->AddStructureType(
        "Nested", // Name
        "", // Description
        ::top::inner::Uuid_Nested // UUID
    );

    // Register the Fields of the Class
    type->AddField(
        "inner", // Name
        "", // Description
        ::top::inner::Uuid_Payload, // Type UUID
        offsetof(Nested, inner), // Field offset
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        false // Output
    );
    type->AddField(
        "ok", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Bool, // Type UUID
        offsetof(Nested, ok), // Field offset
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        false // Output
    );
}
} // namespace top::inner
