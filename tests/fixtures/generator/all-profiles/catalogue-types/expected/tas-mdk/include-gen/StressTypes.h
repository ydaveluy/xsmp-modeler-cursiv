// Copyright line one
// Copyright line two
// ---------------------------------------------------------------------------
// File Name    : StressTypes.h
// ---------------------------------------------------------------------------
/// @file StressTypes.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef STRESSTYPES_H_
#define STRESSTYPES_H_

// ----------------------------------------------------------------------------
// --------------------------------- Includes ---------------------------------
// ----------------------------------------------------------------------------
#include <Smp/ISimulator.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <Smp/Version.h>

// Entry points for static library
extern "C" {
    /// Initialise Package StressTypes.
    /// @param simulator Simulator for registration of factories.
    /// @param typeRegistry Type Registry for registration of types.
    /// @return True if initialisation was successful, false otherwise.
    bool Initialise_StressTypes(
        ::Smp::ISimulator* simulator,
        ::Smp::Publication::ITypeRegistry* typeRegistry);

    /// Finalise Package StressTypes.
    /// @return True if finalisation was successful, false otherwise.
    #if ECSS_SMP_VERSION < 202503L
    bool Finalise_StressTypes();
    #else
    bool Finalise_StressTypes(::Smp::ISimulator *simulator);
    #endif
}

#endif // STRESSTYPES_H_
