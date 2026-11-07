// -----------------------------------------------------------------------------
// File Name    : Demo.cpp
// -----------------------------------------------------------------------------
/// @file Demo.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <Demo.h>
#include <esa/ecss/smp/cdk/Factory.h>
#include <n/A.h>
#include <n/B.h>
#include <n/Device.h>
#include <n/S.h>
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
    /// Initialise Package Demo.
    /// @param simulator Simulator for registration of factories.
    /// @param typeRegistry Type Registry for registration of types.
    /// @return True if initialisation was successful, false otherwise.
    bool Initialise_Demo(
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

        // Register factory for Model Device
        simulator->RegisterFactory(new ::esa::ecss::smp::cdk::Factory<::n::Device>(
            "Device", // Name
            "", // Description
            simulator, // Simulator
            ::n::Uuid_Device // UUID
        ));

        // Register factory for Model A
        simulator->RegisterFactory(new ::esa::ecss::smp::cdk::Factory<::n::A>(
            "A", // Name
            "", // Description
            simulator, // Simulator
            ::n::Uuid_A // UUID
        ));

        // Register factory for Model B
        simulator->RegisterFactory(new ::esa::ecss::smp::cdk::Factory<::n::B>(
            "B", // Name
            "", // Description
            simulator, // Simulator
            ::n::Uuid_B // UUID
        ));

        // Register Service S
        simulator->AddService( new ::n::S(
            "S", // Name
            "", // Description
            simulator, // Parent
            simulator // Simulator
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
    /// Finalise Package Demo.
    /// @param simulator Simulator to finalise.
    /// @return True if finalisation was successful, false otherwise.
    bool Finalise_Demo(::Smp::ISimulator* simulator) {
        // avoid double finalisation of that simulator
        if (::simulators.erase(simulator) == 0) {
            return true;
        }
        return true;
    }

    /// Get the revision of the SMP standard Package Demo was
    /// built against.
    /// @return The revision of the SMP standard.
    ::Smp::UInt64 GetSmpVersion_Demo() {
        return ECSS_SMP_VERSION;
    }
#else
    /// Finalise Package Demo.
    /// @return True if finalisation was successful, false otherwise.
    bool Finalise_Demo() {
        // avoid double finalisation
        if (::simulators.empty()) {
            return true;
        }
        ::simulators.clear();

        return true;
    }
#endif
}
