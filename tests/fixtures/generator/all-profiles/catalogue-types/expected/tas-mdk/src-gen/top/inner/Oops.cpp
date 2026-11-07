// Copyright line one
// Copyright line two
// -----------------------------------------------------------------------------
// File Name    : Oops.cpp
// -----------------------------------------------------------------------------
/// @file top/inner/Oops.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <cstddef>
#include <Smp/Publication/IClassType.h>
#include <top/inner/Oops.h>

/// Top namespace doc.
namespace top
{
/// Inner namespace doc.
///  Second line.
namespace inner
{
void Oops::_Register(::Smp::Publication::ITypeRegistry* registry)
{
    auto *type = registry->AddClassType(
        "Oops", // Name
        "", // Description
        ::top::inner::Uuid_Oops, // UUID
        ::Smp::Uuids::Uuid_Void // Base Class UUID
    );

    // Register the Fields of the Class
    type->AddField(
        "code", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        offsetof(Oops, code), // Field offset
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        false // Output
    );
}
} // namespace inner
} // namespace top
