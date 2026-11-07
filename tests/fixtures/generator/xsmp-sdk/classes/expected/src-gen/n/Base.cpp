// -----------------------------------------------------------------------------
// File Name    : Base.cpp
// -----------------------------------------------------------------------------
/// @file n/Base.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <cstddef>
#include <n/Base.h>
#include <Smp/Publication/IClassType.h>

namespace n
{
void Base::_Register(::Smp::Publication::ITypeRegistry* registry)
{
    auto *type = registry->AddClassType(
        "Base", // Name
        "", // Description
        ::n::Uuid_Base, // UUID
        ::Smp::Uuids::Uuid_Void // Base Class UUID
    );

    // Register the Fields of the Class
    type->AddField(
        "x", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        offsetof(Base, x), // Field offset
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        false // Output
    );
}
} // namespace n
