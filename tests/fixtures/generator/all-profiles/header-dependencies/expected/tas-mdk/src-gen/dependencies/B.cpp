// -----------------------------------------------------------------------------
// File Name    : B.cpp
// -----------------------------------------------------------------------------
/// @file dependencies/B.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <cstddef>
#include <dependencies/A.h>
#include <dependencies/B.h>
#include <Smp/Publication/IClassType.h>
#include <Smp/Publication/IPublishOperation.h>

namespace dependencies
{
void B::_Register(::Smp::Publication::ITypeRegistry* registry)
{
    registry->AddClassType(
        "B", // Name
        "", // Description
        ::dependencies::Uuid_B, // UUID
        ::Smp::Uuids::Uuid_Void // Base Class UUID
    );
}
} // namespace dependencies
