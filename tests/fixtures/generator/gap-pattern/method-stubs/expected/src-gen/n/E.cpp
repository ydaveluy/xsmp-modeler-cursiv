// -----------------------------------------------------------------------------
// File Name    : E.cpp
// -----------------------------------------------------------------------------
/// @file n/E.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <n/E.h>
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
void _Register_E(::Smp::Publication::ITypeRegistry* registry) {
    auto *type = registry->AddEnumerationType(
        "E", // Name
        "", // Description
        ::n::Uuid_E // UUID
    #if ECSS_SMP_VERSION < 202503L
        , sizeof(::n::E) // Size
    #endif
    );

    // Register the Literals of the Enumeration
    type->AddLiteral("A", "", static_cast<::Smp::Int32>(::n::E::A));
}
} // namespace n
