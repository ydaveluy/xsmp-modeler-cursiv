// Copyright line one
// Copyright line two
// -----------------------------------------------------------------------------
// File Name    : Uses.cpp
// -----------------------------------------------------------------------------
/// @file top/other/Uses.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <cstddef>
#include <Smp/Publication/IStructureType.h>
#include <Smp/ViewKind.h>
#include <top/other/Uses.h>

/// Top namespace doc.
namespace top
{
namespace other
{
constexpr ::Smp::Int32 Uses::V;

void Uses::_Register(::Smp::Publication::ITypeRegistry* registry)
{
    auto *type = registry->AddStructureType(
        "Uses", // Name
        "", // Description
        ::top::other::Uuid_Uses // UUID
    );

    // Register the Fields of the Class
    type->AddField(
        "p", // Name
        "", // Description
        ::top::inner::Uuid_Payload, // Type UUID
        offsetof(Uses, p), // Field offset
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        false // Output
    );
    type->AddField(
        "c", // Name
        "", // Description
        ::top::inner::Uuid_Colour, // Type UUID
        offsetof(Uses, c), // Field offset
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        false // Output
    );
}
} // namespace other
} // namespace top
