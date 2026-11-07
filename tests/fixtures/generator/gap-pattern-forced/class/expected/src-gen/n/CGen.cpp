// -----------------------------------------------------------------------------
// File Name    : CGen.cpp
// -----------------------------------------------------------------------------
/// @file n/CGen.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <cstddef>
#include <n/C.h>
#include <Smp/Publication/IClassType.h>

namespace n
{
void CGen::_Register(::Smp::Publication::ITypeRegistry* registry)
{
    registry->AddClassType(
        "C", // Name
        "", // Description
        ::n::Uuid_C, // UUID
        ::Smp::Uuids::Uuid_Void // Base Class UUID
    );
}
} // namespace n
