// -----------------------------------------------------------------------------
// File Name    : Derived.cpp
// -----------------------------------------------------------------------------
/// @file n/Derived.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <cstddef>
#include <n/Derived.h>
#include <Smp/Publication/IClassType.h>

namespace n
{
void Derived::_Register(::Smp::Publication::ITypeRegistry* registry)
{
    registry->AddClassType(
        "Derived", // Name
        "", // Description
        ::n::Uuid_Derived, // UUID
        ::n::Uuid_Base // Base Class UUID
    );
}
} // namespace n
