// Copyright line one
// Copyright line two
// -----------------------------------------------------------------------------
// File Name    : Parent.cpp
// -----------------------------------------------------------------------------
/// @file top/other/Parent.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <cstddef>
#include <Smp/Publication/IClassType.h>
#include <top/other/Parent.h>

/// Top namespace doc.
namespace top
{
namespace other
{
void Parent::_Register(::Smp::Publication::ITypeRegistry* registry)
{
    registry->AddClassType(
        "Parent", // Name
        "", // Description
        ::top::other::Uuid_Parent, // UUID
        ::Smp::Uuids::Uuid_Void // Base Class UUID
    );
}
} // namespace other
} // namespace top
