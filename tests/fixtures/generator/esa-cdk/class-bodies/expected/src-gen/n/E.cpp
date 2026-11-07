// -----------------------------------------------------------------------------
// File Name    : E.cpp
// -----------------------------------------------------------------------------
/// @file n/E.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <cstddef>
#include <n/E.h>
#include <Smp/Publication/IClassType.h>

namespace n
{
void E::_Register(::Smp::Publication::ITypeRegistry* registry)
{
    registry->AddClassType(
        "E", // Name
        "", // Description
        ::n::Uuid_E, // UUID
        ::Smp::Uuids::Uuid_Void // Base Class UUID
    );
}
} // namespace n
