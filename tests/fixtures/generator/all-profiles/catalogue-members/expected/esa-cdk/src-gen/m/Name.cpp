// Block notice
// line two
// -----------------------------------------------------------------------------
// File Name    : Name.cpp
// -----------------------------------------------------------------------------
/// @file m/Name.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <m/Name.h>
#include <Smp/Publication/IType.h>

namespace m
{
void _Register_Name(::Smp::Publication::ITypeRegistry* registry) {
    registry->AddStringType(
        "Name", // Name
        "", // Description
        ::m::Uuid_Name, // UUID
        8 // Length
    );
}
} // namespace m
