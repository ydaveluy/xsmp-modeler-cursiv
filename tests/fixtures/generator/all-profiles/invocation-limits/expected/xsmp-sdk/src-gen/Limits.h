// ---------------------------------------------------------------------------
// File Name    : Limits.h
// ---------------------------------------------------------------------------
/// @file Limits.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef LIMITS_H_
#define LIMITS_H_

// ----------------------------------------------------------------------------
// --------------------------------- Includes ---------------------------------
// ----------------------------------------------------------------------------
#include <Smp/ISimulator.h>
#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/ITypeRegistry.h>

#if defined(__has_include)
#if __has_include(<Smp/Version.h>)
#include <Smp/Version.h>
#endif
#endif

#ifndef ECSS_SMP_VERSION
#define ECSS_SMP_VERSION 202003L
#endif

// Entry points for static library
extern "C" {
    /// Initialise Package Limits.
    /// @param simulator Simulator for registration of factories.
    /// @param typeRegistry Type Registry for registration of types.
    /// @return True if initialisation was successful, false otherwise.
    bool Initialise_Limits(
        ::Smp::ISimulator* simulator,
        ::Smp::Publication::ITypeRegistry* typeRegistry);

#if ECSS_SMP_VERSION >= 202503L
    /// Finalise Package Limits.
    /// @param simulator Simulator to finalise.
    /// @return True if finalisation was successful, false otherwise.
    bool Finalise_Limits(::Smp::ISimulator* simulator);

    /// Get the revision of the SMP standard Package Limits was
    /// built against.
    /// @return The revision of the SMP standard.
    ::Smp::UInt64 GetSmpVersion_Limits();
#else
    /// Finalise Package Limits.
    /// @return True if finalisation was successful, false otherwise.
    bool Finalise_Limits();
#endif
}

#endif // LIMITS_H_
