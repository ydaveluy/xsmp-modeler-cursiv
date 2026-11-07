// -----------------------------------------------------------------------------
// File Name    : B.cpp
// -----------------------------------------------------------------------------
/// @file n/B.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <cstddef>
#include <n/A.h>
#include <n/B.h>
#include <Smp/Publication/IClassType.h>

namespace n
{
void B::_Register(::Smp::Publication::ITypeRegistry* registry)
{
    registry->AddClassType(
        "B", // Name
        "", // Description
        ::n::Uuid_B, // UUID
        ::Smp::Uuids::Uuid_Void // Base Class UUID
    );
}
} // namespace n
