// -----------------------------------------------------------------------------
// File Name    : D.cpp
// -----------------------------------------------------------------------------
/// @file n/D.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <cstddef>
#include <n/D.h>
#include <Smp/Publication/IClassType.h>

namespace n
{
void D::_Register(::Smp::Publication::ITypeRegistry* registry)
{
    registry->AddClassType(
        "D", // Name
        "", // Description
        ::n::Uuid_D, // UUID
        ::Smp::Uuids::Uuid_Void // Base Class UUID
    );
}
} // namespace n
