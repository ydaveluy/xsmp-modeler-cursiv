// The attribute is only metadata: it changes nothing in the generated code.
// -----------------------------------------------------------------------------
// File Name    : Plain.cpp
// -----------------------------------------------------------------------------
/// @file n/Plain.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <cstddef>
#include <n/Plain.h>
#include <Smp/Publication/IClassType.h>

namespace n
{
void Plain::_Register(::Smp::Publication::ITypeRegistry* registry)
{
    registry->AddClassType(
        "Plain", // Name
        "", // Description
        ::n::Uuid_Plain, // UUID
        ::Smp::Uuids::Uuid_Void // Base Class UUID
    );
}
} // namespace n
