// -----------------------------------------------------------------------------
// File Name    : ConstructedGen.cpp
// -----------------------------------------------------------------------------
/// @file n/ConstructedGen.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <cstddef>
#include <n/Constructed.h>
#include <Smp/Publication/IClassType.h>

namespace n
{
void ConstructedGen::_Register(::Smp::Publication::ITypeRegistry* registry)
{
    registry->AddClassType(
        "Constructed", // Name
        "", // Description
        ::n::Uuid_Constructed, // UUID
        ::Smp::Uuids::Uuid_Void // Base Class UUID
    );
}
} // namespace n
