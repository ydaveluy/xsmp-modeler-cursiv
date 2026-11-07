// -----------------------------------------------------------------------------
// File Name    : Limits.cpp
// -----------------------------------------------------------------------------
/// @file Limits.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <esa/ecss/smp/cdk/Factory.h>
#include <Limits.h>
#include <limits/Child.h>
#include <limits/OrdinaryClass.h>
#include <limits/Probe.h>
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
    /// Initialise Package Limits.
    /// @param simulator Simulator for registration of factories.
    /// @param typeRegistry Type Registry for registration of types.
    /// @return True if initialisation was successful, false otherwise.
    bool Initialise_Limits(
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

        // register Class OrdinaryClass
        ::limits::OrdinaryClass::_Register(typeRegistry);

        // Register factory for Model Probe
        simulator->RegisterFactory(new ::esa::ecss::smp::cdk::Factory<::limits::Probe>(
            "Probe", // Name
            "", // Description
            simulator, // Simulator
            ::limits::Uuid_Probe // UUID
        ));

        // Register factory for Model Child
        simulator->RegisterFactory(new ::esa::ecss::smp::cdk::Factory<::limits::Child>(
            "Child", // Name
            "", // Description
            simulator, // Simulator
            ::limits::Uuid_Child // UUID
        ));

        return true;
    }
}

// ---------------------------------------------------------------------------------
// ---------------------------- Finalise Function ------------------------------
// ---------------------------------------------------------------------------------

extern "C"
{
#if ECSS_SMP_VERSION >= 202503L
    /// Finalise Package Limits.
    /// @param simulator Simulator to finalise.
    /// @return True if finalisation was successful, false otherwise.
    bool Finalise_Limits(::Smp::ISimulator* simulator) {
        // avoid double finalisation of that simulator
        if (::simulators.erase(simulator) == 0) {
            return true;
        }
        return true;
    }

    /// Get the revision of the SMP standard Package Limits was
    /// built against.
    /// @return The revision of the SMP standard.
    ::Smp::UInt64 GetSmpVersion_Limits() {
        return ECSS_SMP_VERSION;
    }
#else
    /// Finalise Package Limits.
    /// @return True if finalisation was successful, false otherwise.
    bool Finalise_Limits() {
        // avoid double finalisation
        if (::simulators.empty()) {
            return true;
        }
        ::simulators.clear();

        return true;
    }
#endif
}
