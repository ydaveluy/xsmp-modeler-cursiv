// -----------------------------------------------------------------------------
// File Name    : A_Catalogue_With_A_Name_Longer_Than_The_Small_String_Optimization.pkg.cpp
// -----------------------------------------------------------------------------
/// @file A_Catalogue_With_A_Name_Longer_Than_The_Small_String_Optimization.pkg.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// -----------------------------------------------------------------------------
// --------------------------------- Includes ----------------------------------
// -----------------------------------------------------------------------------
#include <A_Catalogue_With_A_Name_Longer_Than_The_Small_String_Optimization.h>
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

#ifdef WIN32
#define DLL_EXPORT __declspec(dllexport) // %RELAX<mconst> Visual Studio requires a define
#else
#define DLL_EXPORT
#endif

// -----------------------------------------------------------------------------
// -------------------------- Initialise Function ------------------------------
// -----------------------------------------------------------------------------

extern "C" {
    /// Global Initialise function of Package A_Catalogue_With_A_Name_Longer_Than_The_Small_String_Optimization.
    /// @param simulator Simulator for registration of factories.
    /// @param typeRegistry Type Registry for registration of types.
    /// @return True if initialisation was successful, false otherwise.
    DLL_EXPORT bool Initialise(
            ::Smp::ISimulator* simulator,
            ::Smp::Publication::ITypeRegistry* typeRegistry) {
        return Initialise_A_Catalogue_With_A_Name_Longer_Than_The_Small_String_Optimization(simulator, typeRegistry);
    }
}

// -----------------------------------------------------------------------------
// ---------------------------- Finalise Function ------------------------------
// -----------------------------------------------------------------------------

extern "C" {
    /// Global Finalise function of Package A_Catalogue_With_A_Name_Longer_Than_The_Small_String_Optimization.
    /// @param simulator Simulator.
    /// @return True if finalisation was successful, false otherwise.
    DLL_EXPORT bool Finalise(::Smp::ISimulator* simulator) {
#if ECSS_SMP_VERSION >= 202503L
        return Finalise_A_Catalogue_With_A_Name_Longer_Than_The_Small_String_Optimization(simulator);
#else
        static_cast<void>(simulator);
        return Finalise_A_Catalogue_With_A_Name_Longer_Than_The_Small_String_Optimization();
#endif
    }
}

// -----------------------------------------------------------------------------
// ------------------------- GetSmpVersion Function ----------------------------
// -----------------------------------------------------------------------------

#if ECSS_SMP_VERSION >= 202503L
extern "C" {
    /// Global GetSmpVersion function of Package A_Catalogue_With_A_Name_Longer_Than_The_Small_String_Optimization.
    /// The simulator refuses to load a package built against another
    /// revision of the standard than its own.
    /// @return The revision of the SMP standard this package was built
    /// against.
    DLL_EXPORT ::Smp::UInt64 GetSmpVersion() {
        return GetSmpVersion_A_Catalogue_With_A_Name_Longer_Than_The_Small_String_Optimization();
    }
}
#endif
