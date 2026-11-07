// -----------------------------------------------------------------------------
// File Name    : demo.cpp
// -----------------------------------------------------------------------------
/// @file demo.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <demo.h>
#include <n/Grid.h>
#include <n/Items.h>
#include <n/OutputDefaults.h>
#include <n/Pair.h>
#include <n/Row.h>
#include <n/Single.h>
#include <n/TimeFields.h>
#include <n/WithOutput.h>
#include <n/Wrapping.h>
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

        // register Structure WithOutput
        ::n::WithOutput::_Register(typeRegistry);
        // register Structure Single
        ::n::Single::_Register(typeRegistry);
        // register Array Row
        ::n::_Register_Row(typeRegistry);
        // register Array Items
        ::n::_Register_Items(typeRegistry);
        // register Array Pair
        ::n::_Register_Pair(typeRegistry);
        // register Array Grid
        ::n::_Register_Grid(typeRegistry);

        // Register factory for Model TimeFields
        simulator->RegisterFactory(::Xsmp::Factory::Create<::n::TimeFields>(
            "TimeFields", // Name
            "", // Description
            simulator, // Simulator
            ::n::Uuid_TimeFields // UUID
        ));

        // Register factory for Model Wrapping
        simulator->RegisterFactory(::Xsmp::Factory::Create<::n::Wrapping>(
            "Wrapping", // Name
            "", // Description
            simulator, // Simulator
            ::n::Uuid_Wrapping // UUID
        ));

        // Register factory for Model OutputDefaults
        simulator->RegisterFactory(::Xsmp::Factory::Create<::n::OutputDefaults>(
            "OutputDefaults", // Name
            "", // Description
            simulator, // Simulator
            ::n::Uuid_OutputDefaults // UUID
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
