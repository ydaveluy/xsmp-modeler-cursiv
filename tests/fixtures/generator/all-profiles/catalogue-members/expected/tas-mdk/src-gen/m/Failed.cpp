// Block notice
// line two
// -----------------------------------------------------------------------------
// File Name    : Failed.cpp
// -----------------------------------------------------------------------------
/// @file m/Failed.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <cstddef>
#include <m/Failed.h>
#include <Smp/Publication/IClassType.h>

namespace m
{
void Failed::_Register(::Smp::Publication::ITypeRegistry* registry)
{
    registry->AddClassType(
        "Failed", // Name
        "", // Description
        ::m::Uuid_Failed, // UUID
        ::Smp::Uuids::Uuid_Void // Base Class UUID
    );
}
} // namespace m
