// -----------------------------------------------------------------------------
// File Name    : demo.cpp
// -----------------------------------------------------------------------------
/// @file demo.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <demo.h>
#include <n/B.h>
#include <n/Child.h>
#include <n/Clock.h>
#include <n/Sensor.h>
#include <n/T.h>
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
    /// Initialise Package demo.
    /// @param simulator Simulator for registration of factories.
    /// @param typeRegistry Type Registry for registration of types.
    /// @return True if initialisation was successful, false otherwise.
    bool Initialise_demo(
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

        // Register factory for Model Child
        simulator->RegisterFactory(::Xsmp::Factory::Create<::n::Child>(
            "Child", // Name
            "", // Description
            simulator, // Simulator
            ::n::Uuid_Child // UUID
        ));

        // Register factory for Model Sensor
        simulator->RegisterFactory(::Xsmp::Factory::Create<::n::Sensor>(
            "Sensor", // Name
            "", // Description
            simulator, // Simulator
            ::n::Uuid_Sensor // UUID
        ));

        // Register Service Clock
        simulator->AddService( new ::n::Clock(
            "Clock", // Name
            "", // Description
            simulator, // Parent
            simulator // Simulator
        ));

        // Register factory for Model B
        simulator->RegisterFactory(::Xsmp::Factory::Create<::n::B>(
            "B", // Name
            "", // Description
            simulator, // Simulator
            ::n::Uuid_B // UUID
        ));

        // Register Service T
        simulator->AddService( new ::n::T(
            "T", // Name
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
    /// Finalise Package demo.
    /// @param simulator Simulator to finalise.
    /// @return True if finalisation was successful, false otherwise.
    bool Finalise_demo(::Smp::ISimulator* simulator) {
        // avoid double finalisation of that simulator
        if (::simulators.erase(simulator) == 0) {
            return true;
        }
        return true;
    }

    /// Get the revision of the SMP standard Package demo was
    /// built against.
    /// @return The revision of the SMP standard.
    ::Smp::UInt64 GetSmpVersion_demo() {
        return ECSS_SMP_VERSION;
    }
#else
    /// Finalise Package demo.
    /// @return True if finalisation was successful, false otherwise.
    bool Finalise_demo() {
        // avoid double finalisation
        if (::simulators.empty()) {
            return true;
        }
        ::simulators.clear();

        return true;
    }
#endif
}
