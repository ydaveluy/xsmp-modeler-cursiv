// ---------------------------------------------------------------------------
// File Name    : AssociationCycle.h
// ---------------------------------------------------------------------------
/// @file AssociationCycle.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef ASSOCIATIONCYCLE_H_
#define ASSOCIATIONCYCLE_H_

// ----------------------------------------------------------------------------
// --------------------------------- Includes ---------------------------------
// ----------------------------------------------------------------------------
#include <Smp/ISimulator.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <Smp/Version.h>

// Entry points for static library
extern "C" {
    /// Initialise Package AssociationCycle.
    /// @param simulator Simulator for registration of factories.
    /// @param typeRegistry Type Registry for registration of types.
    /// @return True if initialisation was successful, false otherwise.
    bool Initialise_AssociationCycle(
        ::Smp::ISimulator* simulator,
        ::Smp::Publication::ITypeRegistry* typeRegistry);

    /// Finalise Package AssociationCycle.
    /// @return True if finalisation was successful, false otherwise.
    #if ECSS_SMP_VERSION < 202503L
    bool Finalise_AssociationCycle();
    #else
    bool Finalise_AssociationCycle(::Smp::ISimulator *simulator);
    #endif
}

#endif // ASSOCIATIONCYCLE_H_
