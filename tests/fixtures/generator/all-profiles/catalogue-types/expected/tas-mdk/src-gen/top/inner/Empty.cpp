// Copyright line one
// Copyright line two
// -----------------------------------------------------------------------------
// File Name    : Empty.cpp
// -----------------------------------------------------------------------------
/// @file top/inner/Empty.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <cstddef>
#include <Smp/Publication/IStructureType.h>
#include <Smp/ViewKind.h>
#include <top/inner/Empty.h>

/// Top namespace doc.
namespace top
{
/// Inner namespace doc.
///  Second line.
namespace inner
{
void Empty::_Register(::Smp::Publication::ITypeRegistry* registry)
{
    registry->AddStructureType(
        "Empty", // Name
        "", // Description
        ::top::inner::Uuid_Empty // UUID
    );
}
} // namespace inner
} // namespace top
