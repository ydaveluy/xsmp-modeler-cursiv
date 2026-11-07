// -----------------------------------------------------------------------------
// File Name    : T2.cpp
// -----------------------------------------------------------------------------
/// @file c/T2.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <c/T2.h>
#include <cstddef>
#include <Smp/Publication/IStructureType.h>
#include <Smp/ViewKind.h>

namespace c
{
void T2::_Register(::Smp::Publication::ITypeRegistry* registry)
{
    registry->AddStructureType(
        "T2", // Name
        "Doc then line comment on a type.", // Description
        ::c::Uuid_T2 // UUID
    );
}
} // namespace c
