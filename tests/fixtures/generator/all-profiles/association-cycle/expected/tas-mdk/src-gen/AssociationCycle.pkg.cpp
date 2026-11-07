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
#include <Smp/Publication/ITypeRegistry.h>

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
    #if ECSS_SMP_VERSION < 202503L
    DLL_EXPORT bool Finalise(::Smp::ISimulator*) {
        return Finalise_AssociationCycle();
    }
    #else
    DLL_EXPORT bool Finalise(::Smp::ISimulator* simulator) {
        return Finalise_AssociationCycle(simulator);
    }
    #endif
}

// -----------------------------------------------------------------------------
// ---------------------------- GetSmpVersion Function -------------------------
// -----------------------------------------------------------------------------

extern "C" {
    /// Global GetSmpVersion function of Package AssociationCycle.
    /// @return the package smp version.
    DLL_EXPORT uint64_t GetSmpVersion() {
        return ECSS_SMP_VERSION;
    }
}
