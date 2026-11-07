// ---------------------------------------------------------------------------
// File Name    : Demo.h
// ---------------------------------------------------------------------------
/// @file Demo.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef DEMO_H_
#define DEMO_H_

// ----------------------------------------------------------------------------
// --------------------------------- Includes ---------------------------------
// ----------------------------------------------------------------------------
#include <Smp/ISimulator.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <Smp/Version.h>

// Entry points for static library
extern "C" {
    /// Initialise Package Demo.
    /// @param simulator Simulator for registration of factories.
    /// @param typeRegistry Type Registry for registration of types.
    /// @return True if initialisation was successful, false otherwise.
    bool Initialise_Demo(
        ::Smp::ISimulator* simulator,
        ::Smp::Publication::ITypeRegistry* typeRegistry);

    /// Finalise Package Demo.
    /// @return True if finalisation was successful, false otherwise.
    #if ECSS_SMP_VERSION < 202503L
    bool Finalise_Demo();
    #else
    bool Finalise_Demo(::Smp::ISimulator *simulator);
    #endif
}

#endif // DEMO_H_
