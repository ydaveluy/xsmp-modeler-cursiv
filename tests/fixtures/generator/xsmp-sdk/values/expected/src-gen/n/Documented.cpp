// -----------------------------------------------------------------------------
// File Name    : Documented.cpp
// -----------------------------------------------------------------------------
/// @file n/Documented.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <cstddef>
#include <n/Documented.h>
#include <Smp/Publication/IStructureType.h>
#include <Smp/ViewKind.h>

namespace n
{
void Documented::_Register(::Smp::Publication::ITypeRegistry* registry)
{
    registry->AddStructureType(
        "Documented", // Name
        "A path like C:\\temp\\\n Next line.", // Description
        ::n::Uuid_Documented // UUID
    );
}
} // namespace n
