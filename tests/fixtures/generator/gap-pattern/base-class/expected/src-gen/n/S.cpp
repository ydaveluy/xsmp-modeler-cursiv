// The attribute is only metadata: it changes nothing in the generated code.
// -----------------------------------------------------------------------------
// File Name    : S.cpp
// -----------------------------------------------------------------------------
/// @file n/S.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <cstddef>
#include <n/S.h>
#include <Smp/Publication/IStructureType.h>
#include <Smp/ViewKind.h>

namespace n
{
void S::_Register(::Smp::Publication::ITypeRegistry* registry)
{
    registry->AddStructureType(
        "S", // Name
        "", // Description
        ::n::Uuid_S // UUID
    );
}
} // namespace n
