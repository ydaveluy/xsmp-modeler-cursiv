// -----------------------------------------------------------------------------
// File Name    : A.cpp
// -----------------------------------------------------------------------------
/// @file n/A.cpp
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
void A::_Register(::Smp::Publication::ITypeRegistry* registry)
{
    registry->AddClassType(
        "A", // Name
        "", // Description
        ::n::Uuid_A, // UUID
        ::Smp::Uuids::Uuid_Void // Base Class UUID
    );
}
} // namespace n
