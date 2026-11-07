// -----------------------------------------------------------------------------
// File Name    : Mode.cpp
// -----------------------------------------------------------------------------
/// @file device/Mode.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <device/Mode.h>
#include <Smp/Publication/IEnumerationType.h>

#if defined(__has_include)
#if __has_include(<Smp/Version.h>)
#include <Smp/Version.h>
#endif
#endif

#ifndef ECSS_SMP_VERSION
#define ECSS_SMP_VERSION 202003L
#endif

namespace device
{
void _Register_Mode(::Smp::Publication::ITypeRegistry* registry) {
    auto *type = registry->AddEnumerationType(
        "Mode", // Name
        "", // Description
        ::device::Uuid_Mode // UUID
    #if ECSS_SMP_VERSION < 202503L
        , sizeof(::device::Mode) // Size
    #endif
    );

    // Register the Literals of the Enumeration
    type->AddLiteral("Off", "", static_cast<::Smp::Int32>(::device::Mode::Off));
    type->AddLiteral("On", "", static_cast<::Smp::Int32>(::device::Mode::On));
}
} // namespace device
