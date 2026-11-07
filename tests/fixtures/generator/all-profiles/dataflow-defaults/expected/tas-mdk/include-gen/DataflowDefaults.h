// ---------------------------------------------------------------------------
// File Name    : DataflowDefaults.h
// ---------------------------------------------------------------------------
/// @file DataflowDefaults.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef DATAFLOWDEFAULTS_H_
#define DATAFLOWDEFAULTS_H_

// ----------------------------------------------------------------------------
// --------------------------------- Includes ---------------------------------
// ----------------------------------------------------------------------------
#include <Smp/ISimulator.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <Smp/Version.h>

// Entry points for static library
extern "C" {
    /// Initialise Package DataflowDefaults.
    /// @param simulator Simulator for registration of factories.
    /// @param typeRegistry Type Registry for registration of types.
    /// @return True if initialisation was successful, false otherwise.
    bool Initialise_DataflowDefaults(
        ::Smp::ISimulator* simulator,
        ::Smp::Publication::ITypeRegistry* typeRegistry);

    /// Finalise Package DataflowDefaults.
    /// @return True if finalisation was successful, false otherwise.
    #if ECSS_SMP_VERSION < 202503L
    bool Finalise_DataflowDefaults();
    #else
    bool Finalise_DataflowDefaults(::Smp::ISimulator *simulator);
    #endif
}

#endif // DATAFLOWDEFAULTS_H_
