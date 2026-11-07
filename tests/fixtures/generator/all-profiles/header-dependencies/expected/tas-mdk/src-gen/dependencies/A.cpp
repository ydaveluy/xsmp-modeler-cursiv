// -----------------------------------------------------------------------------
// File Name    : A.cpp
// -----------------------------------------------------------------------------
/// @file dependencies/A.cpp
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
void A::_Register(::Smp::Publication::ITypeRegistry* registry)
{
    registry->AddClassType(
        "A", // Name
        "", // Description
        ::dependencies::Uuid_A, // UUID
        ::Smp::Uuids::Uuid_Void // Base Class UUID
    );
}
} // namespace dependencies
