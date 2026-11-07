// Block notice
// line two
// ---------------------------------------------------------------------------
// File Name    : StressMembers.h
// ---------------------------------------------------------------------------
/// @file StressMembers.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef STRESSMEMBERS_H_
#define STRESSMEMBERS_H_

// ----------------------------------------------------------------------------
// --------------------------------- Includes ---------------------------------
// ----------------------------------------------------------------------------
#include <Smp/ISimulator.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <Smp/Version.h>

// Entry points for static library
extern "C" {
    /// Initialise Package StressMembers.
    /// @param simulator Simulator for registration of factories.
    /// @param typeRegistry Type Registry for registration of types.
    /// @return True if initialisation was successful, false otherwise.
    bool Initialise_StressMembers(
        ::Smp::ISimulator* simulator,
        ::Smp::Publication::ITypeRegistry* typeRegistry);

    /// Finalise Package StressMembers.
    /// @return True if finalisation was successful, false otherwise.
    #if ECSS_SMP_VERSION < 202503L
    bool Finalise_StressMembers();
    #else
    bool Finalise_StressMembers(::Smp::ISimulator *simulator);
    #endif
}

#endif // STRESSMEMBERS_H_
