// -----------------------------------------------------------------------------
// File Name    : AssociationCycle.pkg.cpp
// -----------------------------------------------------------------------------
/// @file AssociationCycle.pkg.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// -----------------------------------------------------------------------------
// --------------------------------- Includes ----------------------------------
// -----------------------------------------------------------------------------
#include <AssociationCycle.h>
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
    /// Global Initialise function of Package AssociationCycle.
    /// @param simulator Simulator for registration of factories.
    /// @param typeRegistry Type Registry for registration of types.
    /// @return True if initialisation was successful, false otherwise.
    DLL_EXPORT bool Initialise(
            ::Smp::ISimulator* simulator,
            ::Smp::Publication::ITypeRegistry* typeRegistry) {
        return Initialise_AssociationCycle(simulator, typeRegistry);
    }
}

// -----------------------------------------------------------------------------
// ---------------------------- Finalise Function ------------------------------
// -----------------------------------------------------------------------------

extern "C" {
    /// Global Finalise function of Package AssociationCycle.
    /// @param simulator Simulator.
    /// @return True if finalisation was successful, false otherwise.
    DLL_EXPORT bool Finalise(::Smp::ISimulator* simulator) {
#if ECSS_SMP_VERSION >= 202503L
        return Finalise_AssociationCycle(simulator);
#else
        static_cast<void>(simulator);
        return Finalise_AssociationCycle();
#endif
    }
}

// -----------------------------------------------------------------------------
// ------------------------- GetSmpVersion Function ----------------------------
// -----------------------------------------------------------------------------

#if ECSS_SMP_VERSION >= 202503L
extern "C" {
    /// Global GetSmpVersion function of Package AssociationCycle.
    /// The simulator refuses to load a package built against another
    /// revision of the standard than its own.
    /// @return The revision of the SMP standard this package was built
    /// against.
    DLL_EXPORT ::Smp::UInt64 GetSmpVersion() {
        return GetSmpVersion_AssociationCycle();
    }
}
#endif
