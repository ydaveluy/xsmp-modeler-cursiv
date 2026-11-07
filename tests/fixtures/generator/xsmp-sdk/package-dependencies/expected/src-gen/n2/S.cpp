// A field of `other` and an operation of `types` chain their packages. An
// interface has no implementation, so that `interfaces` is not chained.
// -----------------------------------------------------------------------------
// File Name    : S.cpp
// -----------------------------------------------------------------------------
/// @file n2/S.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <cstddef>
#include <n2/S.h>
#include <Smp/Publication/IStructureType.h>
#include <Smp/ViewKind.h>

namespace n2
{
void S::_Register(::Smp::Publication::ITypeRegistry* registry)
{
    auto *type = registry->AddStructureType(
        "S", // Name
        "", // Description
        ::n2::Uuid_S // UUID
    );

    // Register the Fields of the Class
    type->AddField(
        "x", // Name
        "", // Description
        ::n::Uuid_P, // Type UUID
        offsetof(S, x), // Field offset
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        false // Output
    );
}
} // namespace n2
