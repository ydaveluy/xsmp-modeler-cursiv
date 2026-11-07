// ---------------------------------------------------------------------------
// File Name    : OptionalPointerParameters.h
// ---------------------------------------------------------------------------
/// @file OptionalPointerParameters.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef OPTIONALPOINTERPARAMETERS_H_
#define OPTIONALPOINTERPARAMETERS_H_

// ----------------------------------------------------------------------------
// --------------------------------- Includes ---------------------------------
// ----------------------------------------------------------------------------
#include <Smp/ISimulator.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <Smp/Version.h>

// Entry points for static library
extern "C" {
    /// Initialise Package OptionalPointerParameters.
    /// @param simulator Simulator for registration of factories.
    /// @param typeRegistry Type Registry for registration of types.
    /// @return True if initialisation was successful, false otherwise.
    bool Initialise_OptionalPointerParameters(
        ::Smp::ISimulator* simulator,
        ::Smp::Publication::ITypeRegistry* typeRegistry);

    /// Finalise Package OptionalPointerParameters.
    /// @return True if finalisation was successful, false otherwise.
    #if ECSS_SMP_VERSION < 202503L
    bool Finalise_OptionalPointerParameters();
    #else
    bool Finalise_OptionalPointerParameters(::Smp::ISimulator *simulator);
    #endif
}

#endif // OPTIONALPOINTERPARAMETERS_H_
