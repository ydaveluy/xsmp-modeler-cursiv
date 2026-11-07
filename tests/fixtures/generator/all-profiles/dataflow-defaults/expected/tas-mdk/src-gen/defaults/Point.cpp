// -----------------------------------------------------------------------------
// File Name    : Point.cpp
// -----------------------------------------------------------------------------
/// @file defaults/Point.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <cstddef>
#include <defaults/Point.h>
#include <Smp/Publication/IStructureType.h>
#include <Smp/ViewKind.h>

namespace defaults
{
void Point::_Register(::Smp::Publication::ITypeRegistry* registry)
{
    auto *type = registry->AddStructureType(
        "Point", // Name
        "", // Description
        ::defaults::Uuid_Point // UUID
    );

    // Register the Fields of the Class
    type->AddField(
        "x", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        offsetof(Point, x), // Field offset
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        false // Output
    );
    type->AddField(
        "y", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        offsetof(Point, y), // Field offset
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        false // Output
    );
}
} // namespace defaults
