// Copyright line one
// Copyright line two
// -----------------------------------------------------------------------------
// File Name    : Derived.cpp
// -----------------------------------------------------------------------------
/// @file top/inner/Derived.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <cstddef>
#include <Smp/Publication/IClassType.h>
#include <top/inner/Derived.h>

/// Top namespace doc.
namespace top
{
/// Inner namespace doc.
///  Second line.
namespace inner
{
void Derived::_Register(::Smp::Publication::ITypeRegistry* registry)
{
    auto *type = registry->AddClassType(
        "Derived", // Name
        "", // Description
        ::top::inner::Uuid_Derived, // UUID
        ::top::inner::Uuid_Base // Base Class UUID
    );

    // Register the Fields of the Class
    type->AddField(
        "z", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Bool, // Type UUID
        offsetof(Derived, z), // Field offset
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        false // Output
    );
}
} // namespace inner
} // namespace top
