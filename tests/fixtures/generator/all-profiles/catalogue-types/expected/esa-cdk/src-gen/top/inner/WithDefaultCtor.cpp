// Copyright line one
// Copyright line two
// -----------------------------------------------------------------------------
// File Name    : WithDefaultCtor.cpp
// -----------------------------------------------------------------------------
/// @file top/inner/WithDefaultCtor.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <cstddef>
#include <Smp/Publication/IClassType.h>
#include <top/inner/WithDefaultCtor.h>

/// Top namespace doc.
namespace top
{
/// Inner namespace doc.
///  Second line.
namespace inner
{
void WithDefaultCtor::_Register(::Smp::Publication::ITypeRegistry* registry)
{
    registry->AddClassType(
        "WithDefaultCtor", // Name
        "", // Description
        ::top::inner::Uuid_WithDefaultCtor, // UUID
        ::Smp::Uuids::Uuid_Void // Base Class UUID
    );
}
} // namespace inner
} // namespace top
