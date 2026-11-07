// -----------------------------------------------------------------------------
// File Name    : HeaderDependencies.cpp
// -----------------------------------------------------------------------------
/// @file HeaderDependencies.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <dependencies/A.h>
#include <dependencies/B.h>
#include <dependencies/Defaults.h>
#include <dependencies/Value.h>
#include <HeaderDependencies.h>
#include <unordered_set>

// -----------------------------------------------------------------------------
// ----------------------------- Global variables ------------------------------
// -----------------------------------------------------------------------------

namespace {
/// Simulators set.
std::unordered_set<::Smp::ISimulator*> simulators { };
} // namespace

// -----------------------------------------------------------------------------
// --------------------------- Initialise Function -----------------------------
// -----------------------------------------------------------------------------

extern "C"
{
    /// Initialise Package HeaderDependencies.
    /// @param simulator Simulator for registration of factories.
    /// @param typeRegistry Type Registry for registration of types.
    /// @return True if initialisation was successful, false otherwise.
    bool Initialise_HeaderDependencies(
            ::Smp::ISimulator* simulator,
            ::Smp::Publication::ITypeRegistry* typeRegistry) {
        // check simulator validity
        if (!simulator) {
            return false;
        }
        // avoid double initialisation
        if (!::simulators.emplace(simulator).second) {
            return true;
        }

        // register Class A
        ::dependencies::A::_Register(typeRegistry);
        // register Class B
        ::dependencies::B::_Register(typeRegistry);
        // register Structure Value
        ::dependencies::Value::_Register(typeRegistry);
        // register Class Defaults
        ::dependencies::Defaults::_Register(typeRegistry);

        return true;
    }
}

// ---------------------------------------------------------------------------------
// ---------------------------- Finalise Function ------------------------------
// ---------------------------------------------------------------------------------

extern "C"
{
#if ECSS_SMP_VERSION >= 202503L
    /// Finalise Package HeaderDependencies.
    /// @param simulator Simulator to finalise.
    /// @return True if finalisation was successful, false otherwise.
    bool Finalise_HeaderDependencies(::Smp::ISimulator* simulator) {
        // avoid double finalisation of that simulator
        if (::simulators.erase(simulator) == 0) {
            return true;
        }
        return true;
    }

    /// Get the revision of the SMP standard Package HeaderDependencies was
    /// built against.
    /// @return The revision of the SMP standard.
    ::Smp::UInt64 GetSmpVersion_HeaderDependencies() {
        return ECSS_SMP_VERSION;
    }
#else
    /// Finalise Package HeaderDependencies.
    /// @return True if finalisation was successful, false otherwise.
    bool Finalise_HeaderDependencies() {
        // avoid double finalisation
        if (::simulators.empty()) {
            return true;
        }
        ::simulators.clear();

        return true;
    }
#endif
}
