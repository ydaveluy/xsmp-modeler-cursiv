// The attribute is only metadata: it changes nothing in the generated code.
// -----------------------------------------------------------------------------
// File Name    : C.cpp
// -----------------------------------------------------------------------------
/// @file n/C.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <cstddef>
#include <n/C.h>
#include <Smp/Publication/IClassType.h>

namespace n
{
void C::_Register(::Smp::Publication::ITypeRegistry* registry)
{
    registry->AddClassType(
        "C", // Name
        "", // Description
        ::n::Uuid_C, // UUID
        ::Smp::Uuids::Uuid_Void // Base Class UUID
    );
}
} // namespace n
