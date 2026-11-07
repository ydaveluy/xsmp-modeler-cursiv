// -----------------------------------------------------------------------------
// File Name    : InvalidMode.cpp
// -----------------------------------------------------------------------------
/// @file n/InvalidMode.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <cstddef>
#include <n/InvalidMode.h>
#include <Smp/Publication/IClassType.h>

namespace n
{
void InvalidMode::_Register(::Smp::Publication::ITypeRegistry* registry)
{
    registry->AddClassType(
        "InvalidMode", // Name
        "", // Description
        ::n::Uuid_InvalidMode, // UUID
        ::Smp::Uuids::Uuid_Void // Base Class UUID
    );
}
} // namespace n
