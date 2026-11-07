// ---------------------------------------------------------------------------
// File Name    : Trigraphs.h
// ---------------------------------------------------------------------------
/// @file Trigraphs.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef TRIGRAPHS_H_
#define TRIGRAPHS_H_

// ----------------------------------------------------------------------------
// --------------------------------- Includes ---------------------------------
// ----------------------------------------------------------------------------
#include <Smp/ISimulator.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <Smp/Version.h>

// Entry points for static library
extern "C" {
    /// Initialise Package Trigraphs.
    /// @param simulator Simulator for registration of factories.
    /// @param typeRegistry Type Registry for registration of types.
    /// @return True if initialisation was successful, false otherwise.
    bool Initialise_Trigraphs(
        ::Smp::ISimulator* simulator,
        ::Smp::Publication::ITypeRegistry* typeRegistry);

    /// Finalise Package Trigraphs.
    /// @return True if finalisation was successful, false otherwise.
    #if ECSS_SMP_VERSION < 202503L
    bool Finalise_Trigraphs();
    #else
    bool Finalise_Trigraphs(::Smp::ISimulator *simulator);
    #endif
}

#endif // TRIGRAPHS_H_
