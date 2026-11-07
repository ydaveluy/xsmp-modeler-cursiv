// Block notice
// line two
// -----------------------------------------------------------------------------
// File Name    : Mode.cpp
// -----------------------------------------------------------------------------
/// @file m/Mode.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <m/Mode.h>
#include <Smp/Publication/IEnumerationType.h>

namespace m
{
void _Register_Mode(::Smp::Publication::ITypeRegistry* registry) {
    auto *type = registry->AddEnumerationType(
        "Mode", // Name
        "", // Description
        ::m::Uuid_Mode, // UUID
        sizeof(::m::Mode) // Size
    );

    // Register the Literals of the Enumeration
    type->AddLiteral("Off", "", static_cast<::Smp::Int32>(::m::Mode::Off));
    type->AddLiteral("On", "", static_cast<::Smp::Int32>(::m::Mode::On));
}
} // namespace m
