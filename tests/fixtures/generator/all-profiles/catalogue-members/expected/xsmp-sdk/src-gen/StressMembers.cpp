// Block notice
// line two
// -----------------------------------------------------------------------------
// File Name    : StressMembers.cpp
// -----------------------------------------------------------------------------
/// @file StressMembers.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <m/Empty.h>
#include <m/Failed.h>
#include <m/Full.h>
#include <m/Mode.h>
#include <m/Name.h>
#include <m/OnlyOutput.h>
#include <m/Pair.h>
#include <m/Serv.h>
#include <m/ServKid.h>
#include <m/Three.h>
#include <StressMembers.h>
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
    /// Initialise Package StressMembers.
    /// @param simulator Simulator for registration of factories.
    /// @param typeRegistry Type Registry for registration of types.
    /// @return True if initialisation was successful, false otherwise.
    bool Initialise_StressMembers(
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
        ::m::_Register_Mode(typeRegistry);
        // register Structure Pair
        ::m::Pair::_Register(typeRegistry);
        // register Array Three
        ::m::_Register_Three(typeRegistry);
        // register String Name
        ::m::_Register_Name(typeRegistry);
        // register Exception Failed
        ::m::Failed::_Register(typeRegistry);

        // Register factory for Model Full
        simulator->RegisterFactory(::Xsmp::Factory::Create<::m::Full>(
            "Full", // Name
            "", // Description
            simulator, // Simulator
            ::m::Uuid_Full // UUID
        ));

        // Register Service Serv
        simulator->AddService( new ::m::Serv(
            "Serv", // Name
            "A service.", // Description
            simulator, // Parent
            simulator // Simulator
        ));

        // Register Service ServKid
        simulator->AddService( new ::m::ServKid(
            "ServKid", // Name
            "", // Description
            simulator, // Parent
            simulator // Simulator
        ));

        // Register factory for Model Empty
        simulator->RegisterFactory(::Xsmp::Factory::Create<::m::Empty>(
            "Empty", // Name
            "", // Description
            simulator, // Simulator
            ::m::Uuid_Empty // UUID
        ));

        // Register factory for Model OnlyOutput
        simulator->RegisterFactory(::Xsmp::Factory::Create<::m::OnlyOutput>(
            "OnlyOutput", // Name
            "", // Description
            simulator, // Simulator
            ::m::Uuid_OnlyOutput // UUID
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
    /// Finalise Package StressMembers.
    /// @param simulator Simulator to finalise.
    /// @return True if finalisation was successful, false otherwise.
    bool Finalise_StressMembers(::Smp::ISimulator* simulator) {
        // avoid double finalisation of that simulator
        if (::simulators.erase(simulator) == 0) {
            return true;
        }
        return true;
    }

    /// Get the revision of the SMP standard Package StressMembers was
    /// built against.
    /// @return The revision of the SMP standard.
    ::Smp::UInt64 GetSmpVersion_StressMembers() {
        return ECSS_SMP_VERSION;
    }
#else
    /// Finalise Package StressMembers.
    /// @return True if finalisation was successful, false otherwise.
    bool Finalise_StressMembers() {
        // avoid double finalisation
        if (::simulators.empty()) {
            return true;
        }
        ::simulators.clear();

        return true;
    }
#endif
}
