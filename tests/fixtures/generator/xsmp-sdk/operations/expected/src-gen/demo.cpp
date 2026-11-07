// -----------------------------------------------------------------------------
// File Name    : demo.cpp
// -----------------------------------------------------------------------------
/// @file demo.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <demo.h>
#include <n/Defaults.h>
#include <n/Device.h>
#include <n/Documented.h>
#include <n/NullDefaults.h>
#include <n/Overloads.h>
#include <n/Pointers.h>
#include <n/Skipped.h>
#include <n/Strings.h>
#include <n/WithConstructor.h>
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
            ::Smp::Publication::ITypeRegistry* typeRegistry) {
        // check simulator validity
        if (!simulator) {
            return false;
        }
        // avoid double initialisation
        if (!::simulators.emplace(simulator).second) {
            return true;
        }

        // register Structure Skipped
        ::n::Skipped::_Register(typeRegistry);

        // Register factory for Model Device
        simulator->RegisterFactory(::Xsmp::Factory::Create<::n::Device>(
            "Device", // Name
            "", // Description
            simulator, // Simulator
            ::n::Uuid_Device // UUID
        ));

        // Register factory for Model Documented
        simulator->RegisterFactory(::Xsmp::Factory::Create<::n::Documented>(
            "Documented", // Name
            "", // Description
            simulator, // Simulator
            ::n::Uuid_Documented // UUID
        ));

        // Register factory for Model Strings
        simulator->RegisterFactory(::Xsmp::Factory::Create<::n::Strings>(
            "Strings", // Name
            "", // Description
            simulator, // Simulator
            ::n::Uuid_Strings // UUID
        ));

        // Register factory for Model WithConstructor
        simulator->RegisterFactory(::Xsmp::Factory::Create<::n::WithConstructor>(
            "WithConstructor", // Name
            "", // Description
            simulator, // Simulator
            ::n::Uuid_WithConstructor // UUID
        ));

        // Register factory for Model Overloads
        simulator->RegisterFactory(::Xsmp::Factory::Create<::n::Overloads>(
            "Overloads", // Name
            "", // Description
            simulator, // Simulator
            ::n::Uuid_Overloads // UUID
        ));

        // Register factory for Model Defaults
        simulator->RegisterFactory(::Xsmp::Factory::Create<::n::Defaults>(
            "Defaults", // Name
            "", // Description
            simulator, // Simulator
            ::n::Uuid_Defaults // UUID
        ));

        // Register factory for Model NullDefaults
        simulator->RegisterFactory(::Xsmp::Factory::Create<::n::NullDefaults>(
            "NullDefaults", // Name
            "", // Description
            simulator, // Simulator
            ::n::Uuid_NullDefaults // UUID
        ));

        // Register factory for Model Pointers
        simulator->RegisterFactory(::Xsmp::Factory::Create<::n::Pointers>(
            "Pointers", // Name
            "", // Description
            simulator, // Simulator
            ::n::Uuid_Pointers // UUID
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
