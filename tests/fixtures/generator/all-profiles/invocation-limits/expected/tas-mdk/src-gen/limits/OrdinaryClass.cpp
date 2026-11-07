// -----------------------------------------------------------------------------
// File Name    : OrdinaryClass.cpp
// -----------------------------------------------------------------------------
/// @file limits/OrdinaryClass.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <cstddef>
#include <limits/OrdinaryClass.h>
#include <Smp/Publication/IClassType.h>
#include <Smp/Publication/IPublishOperation.h>

namespace limits
{
void OrdinaryClass::_Register(::Smp::Publication::ITypeRegistry* registry)
{
    registry->AddClassType(
        "OrdinaryClass", // Name
        "", // Description
        ::limits::Uuid_OrdinaryClass, // UUID
        ::Smp::Uuids::Uuid_Void // Base Class UUID
    );
}
} // namespace limits
