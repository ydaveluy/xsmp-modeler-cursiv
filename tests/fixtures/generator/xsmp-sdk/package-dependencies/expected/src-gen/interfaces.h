// ---------------------------------------------------------------------------
// File Name    : interfaces.h
// ---------------------------------------------------------------------------
/// @file interfaces.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef INTERFACES_H_
#define INTERFACES_H_

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
    /// Initialise Package interfaces.
    /// @param simulator Simulator for registration of factories.
    /// @param typeRegistry Type Registry for registration of types.
    /// @return True if initialisation was successful, false otherwise.
    bool Initialise_interfaces(
        ::Smp::ISimulator* simulator,
        ::Smp::Publication::ITypeRegistry* typeRegistry);

#if ECSS_SMP_VERSION >= 202503L
    /// Finalise Package interfaces.
    /// @param simulator Simulator to finalise.
    /// @return True if finalisation was successful, false otherwise.
    bool Finalise_interfaces(::Smp::ISimulator* simulator);

    /// Get the revision of the SMP standard Package interfaces was
    /// built against.
    /// @return The revision of the SMP standard.
    ::Smp::UInt64 GetSmpVersion_interfaces();
#else
    /// Finalise Package interfaces.
    /// @return True if finalisation was successful, false otherwise.
    bool Finalise_interfaces();
#endif
}

#endif // INTERFACES_H_
