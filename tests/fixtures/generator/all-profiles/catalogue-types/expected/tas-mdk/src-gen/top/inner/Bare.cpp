// Copyright line one
// Copyright line two
// -----------------------------------------------------------------------------
// File Name    : Bare.cpp
// -----------------------------------------------------------------------------
/// @file top/inner/Bare.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <cstddef>
#include <Smp/Publication/IClassType.h>
#include <top/inner/Bare.h>

/// Top namespace doc.
namespace top
{
/// Inner namespace doc.
///  Second line.
namespace inner
{
void Bare::_Register(::Smp::Publication::ITypeRegistry* registry)
{
    registry->AddClassType(
        "Bare", // Name
        "", // Description
        ::top::inner::Uuid_Bare, // UUID
        ::Smp::Uuids::Uuid_Void // Base Class UUID
    );
}
} // namespace inner
} // namespace top
