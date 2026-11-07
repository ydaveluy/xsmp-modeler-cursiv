// -----------------------------------------------------------------------------
// File Name    : X.cpp
// -----------------------------------------------------------------------------
/// @file n/X.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <cstddef>
#include <n/X.h>
#include <Smp/Publication/IClassType.h>

namespace n
{
void X::_Register(::Smp::Publication::ITypeRegistry* registry)
{
    registry->AddClassType(
        "X", // Name
        "", // Description
        ::n::Uuid_X, // UUID
        ::Smp::Uuids::Uuid_Void // Base Class UUID
    );
}
} // namespace n
