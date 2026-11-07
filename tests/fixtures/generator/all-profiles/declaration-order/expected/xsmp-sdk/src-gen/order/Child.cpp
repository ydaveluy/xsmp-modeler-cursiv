// -----------------------------------------------------------------------------
// File Name    : Child.cpp
// -----------------------------------------------------------------------------
/// @file order/Child.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <cstddef>
#include <order/Child.h>
#include <Smp/Publication/IClassType.h>

namespace order
{
void Child::_Register(::Smp::Publication::ITypeRegistry* registry)
{
    registry->AddClassType(
        "Child", // Name
        "", // Description
        ::order::Uuid_Child, // UUID
        ::order::Uuid_Parent // Base Class UUID
    );
}
} // namespace order
