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
#include <Smp/Version.h>

namespace m
{
void _Register_Mode(::Smp::Publication::ITypeRegistry* registry) {
    auto *type = registry->AddEnumerationType(
        "Mode", // Name
        "", // Description
        #if ECSS_SMP_VERSION < 202503L
        ::m::Uuid_Mode, // UUID
        sizeof(::m::Mode) // Size
        #else
        ::m::Uuid_Mode // UUID
        #endif
    );

    // Register the Literals of the Enumeration
    type->AddLiteral("Off", "", static_cast<::Smp::Int32>(::m::Mode::Off));
    type->AddLiteral("On", "", static_cast<::Smp::Int32>(::m::Mode::On));
}
} // namespace m
