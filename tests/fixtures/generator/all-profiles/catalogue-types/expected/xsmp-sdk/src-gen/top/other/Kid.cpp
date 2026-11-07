// Copyright line one
// Copyright line two
// -----------------------------------------------------------------------------
// File Name    : Kid.cpp
// -----------------------------------------------------------------------------
/// @file top/other/Kid.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <cstddef>
#include <Smp/Publication/IClassType.h>
#include <top/other/Kid.h>

namespace top::other
{
void Kid::_Register(::Smp::Publication::ITypeRegistry* registry)
{
    registry->AddClassType(
        "Kid", // Name
        "Derived declared before its base.", // Description
        ::top::other::Uuid_Kid, // UUID
        ::top::other::Uuid_Parent // Base Class UUID
    );
}
} // namespace top::other
