// -----------------------------------------------------------------------------
// File Name    : OptionalPointerParameters.cpp
// -----------------------------------------------------------------------------
/// @file OptionalPointerParameters.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <optional/Probe.h>
#include <OptionalPointerParameters.h>
#include <unordered_set>
#include <Xsmp/Factory.h>

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
    /// Initialise Package OptionalPointerParameters.
    /// @param simulator Simulator for registration of factories.
    /// @param typeRegistry Type Registry for registration of types.
    /// @return True if initialisation was successful, false otherwise.
    bool Initialise_OptionalPointerParameters(
            ::Smp::ISimulator* simulator,
            [[maybe_unused]] ::Smp::Publication::ITypeRegistry* typeRegistry) {
        // check simulator validity
        if (!simulator) {
            return false;
        }
        // avoid double initialisation
        if (!::simulators.emplace(simulator).second) {
            return true;
        }

        // Register factory for Model Probe
        simulator->RegisterFactory(::Xsmp::Factory::Create<::optional::Probe>(
            "Probe", // Name
            "", // Description
            simulator, // Simulator
            ::optional::Uuid_Probe // UUID
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
    /// Finalise Package OptionalPointerParameters.
    /// @param simulator Simulator to finalise.
    /// @return True if finalisation was successful, false otherwise.
    bool Finalise_OptionalPointerParameters(::Smp::ISimulator* simulator) {
        // avoid double finalisation of that simulator
        if (::simulators.erase(simulator) == 0) {
            return true;
        }
        return true;
    }

    /// Get the revision of the SMP standard Package OptionalPointerParameters was
    /// built against.
    /// @return The revision of the SMP standard.
    ::Smp::UInt64 GetSmpVersion_OptionalPointerParameters() {
        return ECSS_SMP_VERSION;
    }
#else
    /// Finalise Package OptionalPointerParameters.
    /// @return True if finalisation was successful, false otherwise.
    bool Finalise_OptionalPointerParameters() {
        // avoid double finalisation
        if (::simulators.empty()) {
            return true;
        }
        ::simulators.clear();

        return true;
    }
#endif
}
