// -----------------------------------------------------------------------------
// File Name    : Constants.cpp
// -----------------------------------------------------------------------------
/// @file n/Constants.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <cstddef>
#include <n/Constants.h>
#include <Smp/Publication/IStructureType.h>
#include <Smp/ViewKind.h>

namespace n
{
void Constants::_Register(::Smp::Publication::ITypeRegistry* registry)
{
    registry->AddStructureType(
        "Constants", // Name
        "", // Description
        ::n::Uuid_Constants // UUID
    );
}
} // namespace n
