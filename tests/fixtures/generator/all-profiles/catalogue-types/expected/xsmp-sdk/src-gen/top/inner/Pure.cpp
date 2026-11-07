// Copyright line one
// Copyright line two
// -----------------------------------------------------------------------------
// File Name    : Pure.cpp
// -----------------------------------------------------------------------------
/// @file top/inner/Pure.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <cstddef>
#include <Smp/Publication/IClassType.h>
#include <Smp/Publication/IPublishOperation.h>
#include <top/inner/Pure.h>

/// Inner namespace doc.
///  Second line.
namespace top::inner
{
void Pure::_Register(::Smp::Publication::ITypeRegistry* registry)
{
    registry->AddClassType(
        "Pure", // Name
        "A class that cannot be instantiated.", // Description
        ::top::inner::Uuid_Pure, // UUID
        ::Smp::Uuids::Uuid_Void // Base Class UUID
    );
}
} // namespace top::inner
