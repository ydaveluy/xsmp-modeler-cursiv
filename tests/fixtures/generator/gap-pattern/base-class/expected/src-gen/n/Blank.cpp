// The attribute is only metadata: it changes nothing in the generated code.
// -----------------------------------------------------------------------------
// File Name    : Blank.cpp
// -----------------------------------------------------------------------------
/// @file n/Blank.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <cstddef>
#include <n/Blank.h>
#include <Smp/Publication/IClassType.h>

namespace n
{
void Blank::_Register(::Smp::Publication::ITypeRegistry* registry)
{
    registry->AddClassType(
        "Blank", // Name
        "", // Description
        ::n::Uuid_Blank, // UUID
        ::Smp::Uuids::Uuid_Void // Base Class UUID
    );
}
} // namespace n
