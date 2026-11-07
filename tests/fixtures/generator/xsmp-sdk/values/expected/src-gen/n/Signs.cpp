// -----------------------------------------------------------------------------
// File Name    : Signs.cpp
// -----------------------------------------------------------------------------
/// @file n/Signs.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <cstddef>
#include <n/Signs.h>
#include <Smp/Publication/IStructureType.h>
#include <Smp/ViewKind.h>

namespace n
{
void Signs::_Register(::Smp::Publication::ITypeRegistry* registry)
{
    registry->AddStructureType(
        "Signs", // Name
        "", // Description
        ::n::Uuid_Signs // UUID
    );
}
} // namespace n
