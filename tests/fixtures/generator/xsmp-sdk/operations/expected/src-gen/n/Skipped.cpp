// -----------------------------------------------------------------------------
// File Name    : Skipped.cpp
// -----------------------------------------------------------------------------
/// @file n/Skipped.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <cstddef>
#include <n/Skipped.h>
#include <Smp/Publication/IStructureType.h>
#include <Smp/ViewKind.h>

namespace n
{
void Skipped::_Register(::Smp::Publication::ITypeRegistry* registry)
{
    registry->AddStructureType(
        "Skipped", // Name
        "", // Description
        ::n::Uuid_Skipped // UUID
    );
}
} // namespace n
