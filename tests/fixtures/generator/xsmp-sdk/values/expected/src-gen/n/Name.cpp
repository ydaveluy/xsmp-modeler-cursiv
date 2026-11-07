// -----------------------------------------------------------------------------
// File Name    : Name.cpp
// -----------------------------------------------------------------------------
/// @file n/Name.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <n/Name.h>
#include <Smp/Publication/IType.h>

namespace n
{
void _Register_Name(::Smp::Publication::ITypeRegistry* registry) {
    registry->AddStringType(
        "Name", // Name
        "", // Description
        ::n::Uuid_Name, // UUID
        8 // Length
    );
}
} // namespace n
