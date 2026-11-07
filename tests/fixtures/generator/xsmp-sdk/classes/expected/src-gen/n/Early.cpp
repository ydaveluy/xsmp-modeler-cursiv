// -----------------------------------------------------------------------------
// File Name    : Early.cpp
// -----------------------------------------------------------------------------
/// @file n/Early.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <cstddef>
#include <n/Early.h>
#include <Smp/Publication/IClassType.h>

namespace n
{
void Early::_Register(::Smp::Publication::ITypeRegistry* registry)
{
    registry->AddClassType(
        "Early", // Name
        "", // Description
        ::n::Uuid_Early, // UUID
        ::Smp::Uuids::Uuid_Void // Base Class UUID
    );
}
} // namespace n
