// Copyright line one
// Copyright line two
// -----------------------------------------------------------------------------
// File Name    : Colour.cpp
// -----------------------------------------------------------------------------
/// @file top/inner/Colour.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <Smp/Publication/IEnumerationType.h>
#include <top/inner/Colour.h>

#if defined(__has_include)
#if __has_include(<Smp/Version.h>)
#include <Smp/Version.h>
#endif
#endif

#ifndef ECSS_SMP_VERSION
#define ECSS_SMP_VERSION 202003L
#endif

/// Inner namespace doc.
///  Second line.
namespace top::inner
{
void _Register_Colour(::Smp::Publication::ITypeRegistry* registry) {
    auto *type = registry->AddEnumerationType(
        "Colour", // Name
        "An enumeration.", // Description
        ::top::inner::Uuid_Colour // UUID
    #if ECSS_SMP_VERSION < 202503L
        , sizeof(::top::inner::Colour) // Size
    #endif
    );

    // Register the Literals of the Enumeration
    type->AddLiteral("Red", "The red one.", static_cast<::Smp::Int32>(::top::inner::Colour::Red));
    type->AddLiteral("Green", "The green one, it\'s \"fine\".", static_cast<::Smp::Int32>(::top::inner::Colour::Green));
    type->AddLiteral("Blue", "", static_cast<::Smp::Int32>(::top::inner::Colour::Blue));
}
} // namespace top::inner
