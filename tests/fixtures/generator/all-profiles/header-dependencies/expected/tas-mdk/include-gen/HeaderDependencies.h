// ---------------------------------------------------------------------------
// File Name    : HeaderDependencies.h
// ---------------------------------------------------------------------------
/// @file HeaderDependencies.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef HEADERDEPENDENCIES_H_
#define HEADERDEPENDENCIES_H_

// ----------------------------------------------------------------------------
// --------------------------------- Includes ---------------------------------
// ----------------------------------------------------------------------------
#include <Smp/ISimulator.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <Smp/Version.h>

// Entry points for static library
extern "C" {
    /// Initialise Package HeaderDependencies.
    /// @param simulator Simulator for registration of factories.
    /// @param typeRegistry Type Registry for registration of types.
    /// @return True if initialisation was successful, false otherwise.
    bool Initialise_HeaderDependencies(
        ::Smp::ISimulator* simulator,
        ::Smp::Publication::ITypeRegistry* typeRegistry);

    /// Finalise Package HeaderDependencies.
    /// @return True if finalisation was successful, false otherwise.
    #if ECSS_SMP_VERSION < 202503L
    bool Finalise_HeaderDependencies();
    #else
    bool Finalise_HeaderDependencies(::Smp::ISimulator *simulator);
    #endif
}

#endif // HEADERDEPENDENCIES_H_
