// Copyright line one
// Copyright line two
// -----------------------------------------------------------------------------
// File Name    : Oops2.cpp
// -----------------------------------------------------------------------------
/// @file top/inner/Oops2.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <cstddef>
#include <Smp/Publication/IClassType.h>
#include <top/inner/Oops2.h>

/// Inner namespace doc.
///  Second line.
namespace top::inner
{
void Oops2::_Register(::Smp::Publication::ITypeRegistry* registry)
{
    registry->AddClassType(
        "Oops2", // Name
        "", // Description
        ::top::inner::Uuid_Oops2, // UUID
        ::top::inner::Uuid_Oops // Base Class UUID
    );
}
} // namespace top::inner
