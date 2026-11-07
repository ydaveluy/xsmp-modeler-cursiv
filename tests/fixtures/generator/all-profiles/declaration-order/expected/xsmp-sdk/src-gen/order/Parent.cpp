// -----------------------------------------------------------------------------
// File Name    : Parent.cpp
// -----------------------------------------------------------------------------
/// @file order/Parent.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <cstddef>
#include <order/Parent.h>
#include <Smp/Publication/IClassType.h>

namespace order
{
void Parent::_Register(::Smp::Publication::ITypeRegistry* registry)
{
    registry->AddClassType(
        "Parent", // Name
        "", // Description
        ::order::Uuid_Parent, // UUID
        ::Smp::Uuids::Uuid_Void // Base Class UUID
    );
}
} // namespace order
