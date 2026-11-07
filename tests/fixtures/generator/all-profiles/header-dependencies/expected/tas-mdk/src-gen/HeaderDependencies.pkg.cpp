// -----------------------------------------------------------------------------
// File Name    : HeaderDependencies.pkg.cpp
// -----------------------------------------------------------------------------
/// @file HeaderDependencies.pkg.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// -----------------------------------------------------------------------------
// --------------------------------- Includes ----------------------------------
// -----------------------------------------------------------------------------
#include <HeaderDependencies.h>
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
    /// Global Initialise function of Package HeaderDependencies.
    /// @param simulator Simulator for registration of factories.
    /// @param typeRegistry Type Registry for registration of types.
    /// @return True if initialisation was successful, false otherwise.
    DLL_EXPORT bool Initialise(
            ::Smp::ISimulator* simulator,
            ::Smp::Publication::ITypeRegistry* typeRegistry) {
        return Initialise_HeaderDependencies(simulator, typeRegistry);
    }
}

// -----------------------------------------------------------------------------
// ---------------------------- Finalise Function ------------------------------
// -----------------------------------------------------------------------------

extern "C" {
    /// Global Finalise function of Package HeaderDependencies.
    /// @param simulator Simulator.
    /// @return True if finalisation was successful, false otherwise.
    #if ECSS_SMP_VERSION < 202503L
    DLL_EXPORT bool Finalise(::Smp::ISimulator*) {
        return Finalise_HeaderDependencies();
    }
    #else
    DLL_EXPORT bool Finalise(::Smp::ISimulator* simulator) {
        return Finalise_HeaderDependencies(simulator);
    }
    #endif
}

// -----------------------------------------------------------------------------
// ---------------------------- GetSmpVersion Function -------------------------
// -----------------------------------------------------------------------------

extern "C" {
    /// Global GetSmpVersion function of Package HeaderDependencies.
    /// @return the package smp version.
    DLL_EXPORT uint64_t GetSmpVersion() {
        return ECSS_SMP_VERSION;
    }
}
