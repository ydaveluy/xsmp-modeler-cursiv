// -----------------------------------------------------------------------------
// File Name    : Defaults.cpp
// -----------------------------------------------------------------------------
/// @file dependencies/Defaults.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <cstddef>
#include <dependencies/Defaults.h>
#include <Smp/Publication/IClassType.h>
#include <Smp/Publication/IPublishOperation.h>

namespace dependencies
{
void Defaults::_Register(::Smp::Publication::ITypeRegistry* registry)
{
    registry->AddClassType(
        "Defaults", // Name
        "", // Description
        ::dependencies::Uuid_Defaults, // UUID
        ::Smp::Uuids::Uuid_Void // Base Class UUID
    );
}
} // namespace dependencies
