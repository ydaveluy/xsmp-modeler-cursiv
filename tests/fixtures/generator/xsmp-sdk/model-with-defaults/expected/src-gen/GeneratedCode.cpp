// -----------------------------------------------------------------------------
// File Name    : GeneratedCode.cpp
// -----------------------------------------------------------------------------
/// @file GeneratedCode.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <device/Device.h>
#include <device/Mode.h>
#include <device/State.h>
#include <GeneratedCode.h>
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
    /// Initialise Package GeneratedCode.
    /// @param simulator Simulator for registration of factories.
    /// @param typeRegistry Type Registry for registration of types.
    /// @return True if initialisation was successful, false otherwise.
    bool Initialise_GeneratedCode(
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

        // register Enumeration Mode
        ::device::_Register_Mode(typeRegistry);
        // register Structure State
        ::device::State::_Register(typeRegistry);

        // Register factory for Model Device
        simulator->RegisterFactory(::Xsmp::Factory::Create<::device::Device>(
            "Device", // Name
            "", // Description
            simulator, // Simulator
            ::device::Uuid_Device // UUID
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
    /// Finalise Package GeneratedCode.
    /// @param simulator Simulator to finalise.
    /// @return True if finalisation was successful, false otherwise.
    bool Finalise_GeneratedCode(::Smp::ISimulator* simulator) {
        // avoid double finalisation of that simulator
        if (::simulators.erase(simulator) == 0) {
            return true;
        }
        return true;
    }

    /// Get the revision of the SMP standard Package GeneratedCode was
    /// built against.
    /// @return The revision of the SMP standard.
    ::Smp::UInt64 GetSmpVersion_GeneratedCode() {
        return ECSS_SMP_VERSION;
    }
#else
    /// Finalise Package GeneratedCode.
    /// @return True if finalisation was successful, false otherwise.
    bool Finalise_GeneratedCode() {
        // avoid double finalisation
        if (::simulators.empty()) {
            return true;
        }
        ::simulators.clear();

        return true;
    }
#endif
}
