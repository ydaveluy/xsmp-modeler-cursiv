// -----------------------------------------------------------------------------
// File Name    : Mode.cpp
// -----------------------------------------------------------------------------
/// @file n/Mode.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <n/Mode.h>
#include <Smp/Publication/IEnumerationType.h>

#if defined(__has_include)
#if __has_include(<Smp/Version.h>)
#include <Smp/Version.h>
#endif
#endif

#ifndef ECSS_SMP_VERSION
#define ECSS_SMP_VERSION 202003L
#endif

namespace n
{
void _Register_Mode(::Smp::Publication::ITypeRegistry* registry) {
    auto *type = registry->AddEnumerationType(
        "Mode", // Name
        "", // Description
        ::n::Uuid_Mode // UUID
    #if ECSS_SMP_VERSION < 202503L
        , sizeof(::n::Mode) // Size
    #endif
    );

    // Register the Literals of the Enumeration
    type->AddLiteral("Standby", "Idle mode.", static_cast<::Smp::Int32>(::n::Mode::Standby));
    type->AddLiteral("Nominal", "", static_cast<::Smp::Int32>(::n::Mode::Nominal));
    type->AddLiteral("Off", "Component is powered off and fully idle.", static_cast<::Smp::Int32>(::n::Mode::Off));
}
} // namespace n
