// ---------------------------------------------------------------------------
// File Name    : A_Catalogue_With_A_Name_Longer_Than_The_Small_String_Optimization.h
// ---------------------------------------------------------------------------
/// @file A_Catalogue_With_A_Name_Longer_Than_The_Small_String_Optimization.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef A_CATALOGUE_WITH_A_NAME_LONGER_THAN_THE_SMALL_STRING_OPTIMIZATION_H_
#define A_CATALOGUE_WITH_A_NAME_LONGER_THAN_THE_SMALL_STRING_OPTIMIZATION_H_

// ----------------------------------------------------------------------------
// --------------------------------- Includes ---------------------------------
// ----------------------------------------------------------------------------
#include <Smp/ISimulator.h>
#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/ITypeRegistry.h>

#if defined(__has_include)
#if __has_include(<Smp/Version.h>)
#include <Smp/Version.h>
#endif
#endif

#ifndef ECSS_SMP_VERSION
#define ECSS_SMP_VERSION 202003L
#endif

// Entry points for static library
extern "C" {
    /// Initialise Package A_Catalogue_With_A_Name_Longer_Than_The_Small_String_Optimization.
    /// @param simulator Simulator for registration of factories.
    /// @param typeRegistry Type Registry for registration of types.
    /// @return True if initialisation was successful, false otherwise.
    bool Initialise_A_Catalogue_With_A_Name_Longer_Than_The_Small_String_Optimization(
        ::Smp::ISimulator* simulator,
        ::Smp::Publication::ITypeRegistry* typeRegistry);

#if ECSS_SMP_VERSION >= 202503L
    /// Finalise Package A_Catalogue_With_A_Name_Longer_Than_The_Small_String_Optimization.
    /// @param simulator Simulator to finalise.
    /// @return True if finalisation was successful, false otherwise.
    bool Finalise_A_Catalogue_With_A_Name_Longer_Than_The_Small_String_Optimization(::Smp::ISimulator* simulator);

    /// Get the revision of the SMP standard Package A_Catalogue_With_A_Name_Longer_Than_The_Small_String_Optimization was
    /// built against.
    /// @return The revision of the SMP standard.
    ::Smp::UInt64 GetSmpVersion_A_Catalogue_With_A_Name_Longer_Than_The_Small_String_Optimization();
#else
    /// Finalise Package A_Catalogue_With_A_Name_Longer_Than_The_Small_String_Optimization.
    /// @return True if finalisation was successful, false otherwise.
    bool Finalise_A_Catalogue_With_A_Name_Longer_Than_The_Small_String_Optimization();
#endif
}

#endif // A_CATALOGUE_WITH_A_NAME_LONGER_THAN_THE_SMALL_STRING_OPTIMIZATION_H_
