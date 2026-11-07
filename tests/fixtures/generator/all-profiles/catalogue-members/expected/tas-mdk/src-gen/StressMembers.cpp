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
#include <TasMdk/Factory.h>
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
        simulator->RegisterFactory(new ::TasMdk::Factory(
            "Full", // Name
            "", // Description
            simulator, // parent
            ::m::Uuid_Full, // UUID
            "::m::Full", // Type name
            [](::Smp::String8 name,
                ::Smp::String8 description,
                ::Smp::IObject* parent,
                ::Smp::Publication::ITypeRegistry* type_registry) {
                return new ::m::Full(name, description, parent, type_registry);
            }, // Instantiation callback
            typeRegistry // Type Registry
        ));

        // Register Service Serv
        simulator->AddService( new ::m::Serv(
            "Serv", // Name
            "A service.", // Description
            simulator, // Parent
            typeRegistry // Type Registry
        ));

        // Register Service ServKid
        simulator->AddService( new ::m::ServKid(
            "ServKid", // Name
            "", // Description
            simulator, // Parent
            typeRegistry // Type Registry
        ));

        // Register factory for Model Empty
        simulator->RegisterFactory(new ::TasMdk::Factory(
            "Empty", // Name
            "", // Description
            simulator, // parent
            ::m::Uuid_Empty, // UUID
            "::m::Empty", // Type name
            [](::Smp::String8 name,
                ::Smp::String8 description,
                ::Smp::IObject* parent,
                ::Smp::Publication::ITypeRegistry* type_registry) {
                return new ::m::Empty(name, description, parent, type_registry);
            }, // Instantiation callback
            typeRegistry // Type Registry
        ));

        // Register factory for Model OnlyOutput
        simulator->RegisterFactory(new ::TasMdk::Factory(
            "OnlyOutput", // Name
            "", // Description
            simulator, // parent
            ::m::Uuid_OnlyOutput, // UUID
            "::m::OnlyOutput", // Type name
            [](::Smp::String8 name,
                ::Smp::String8 description,
                ::Smp::IObject* parent,
                ::Smp::Publication::ITypeRegistry* type_registry) {
                return new ::m::OnlyOutput(name, description, parent, type_registry);
            }, // Instantiation callback
            typeRegistry // Type Registry
        ));

        return true;
    }
}

// ---------------------------------------------------------------------------------
// ---------------------------- Finalise Function ------------------------------
// ---------------------------------------------------------------------------------

extern "C"
{
    /// Finalise Package StressMembers.
    /// @return True if finalisation was successful, false otherwise.
    #if ECSS_SMP_VERSION < 202503L
    bool Finalise_StressMembers() {
        // avoid double finalisation
        if (::simulators.empty()) {
            return true;
        }
        ::simulators.clear();

        return true;
    }
    #else
    bool Finalise_StressMembers(::Smp::ISimulator *simulator) {
        // avoid double finalisation
        if (::simulators.empty()) {
            return true;
        }
        // check simulator validity
        if (!simulator) {
            return false;
        }
        // avoid double finalisation
        if (!::simulators.erase(simulator)) {
            return true;
        }
        return true;
    }
    #endif
}
