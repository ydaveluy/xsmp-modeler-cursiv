// -----------------------------------------------------------------------------
// File Name    : Demo.cpp
// -----------------------------------------------------------------------------
/// @file Demo.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <Demo.h>
#include <n/Bare.h>
#include <n/Device.h>
#include <n/Invokable.h>
#include <n/Label.h>
#include <n/M.h>
#include <n/NullDefaults.h>
#include <n/State.h>
#include <n/WithParameters.h>
#include <n/WithReturn.h>
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

        // register String Label
        ::n::_Register_Label(typeRegistry);
        // register Structure State
        ::n::State::_Register(typeRegistry);

        // Register factory for Model M
        simulator->RegisterFactory(new ::TasMdk::Factory(
            "M", // Name
            "", // Description
            simulator, // parent
            ::n::Uuid_M, // UUID
            "::n::M", // Type name
            [](::Smp::String8 name,
                ::Smp::String8 description,
                ::Smp::IObject* parent,
                ::Smp::Publication::ITypeRegistry* type_registry) {
                return new ::n::M(name, description, parent, type_registry);
            }, // Instantiation callback
            typeRegistry // Type Registry
        ));

        // Register factory for Model Device
        simulator->RegisterFactory(new ::TasMdk::Factory(
            "Device", // Name
            "", // Description
            simulator, // parent
            ::n::Uuid_Device, // UUID
            "::n::Device", // Type name
            [](::Smp::String8 name,
                ::Smp::String8 description,
                ::Smp::IObject* parent,
                ::Smp::Publication::ITypeRegistry* type_registry) {
                return new ::n::Device(name, description, parent, type_registry);
            }, // Instantiation callback
            typeRegistry // Type Registry
        ));

        // Register factory for Model Invokable
        simulator->RegisterFactory(new ::TasMdk::Factory(
            "Invokable", // Name
            "", // Description
            simulator, // parent
            ::n::Uuid_Invokable, // UUID
            "::n::Invokable", // Type name
            [](::Smp::String8 name,
                ::Smp::String8 description,
                ::Smp::IObject* parent,
                ::Smp::Publication::ITypeRegistry* type_registry) {
                return new ::n::Invokable(name, description, parent, type_registry);
            }, // Instantiation callback
            typeRegistry // Type Registry
        ));

        // Register factory for Model NullDefaults
        simulator->RegisterFactory(new ::TasMdk::Factory(
            "NullDefaults", // Name
            "", // Description
            simulator, // parent
            ::n::Uuid_NullDefaults, // UUID
            "::n::NullDefaults", // Type name
            [](::Smp::String8 name,
                ::Smp::String8 description,
                ::Smp::IObject* parent,
                ::Smp::Publication::ITypeRegistry* type_registry) {
                return new ::n::NullDefaults(name, description, parent, type_registry);
            }, // Instantiation callback
            typeRegistry // Type Registry
        ));

        // Register factory for Model WithParameters
        simulator->RegisterFactory(new ::TasMdk::Factory(
            "WithParameters", // Name
            "", // Description
            simulator, // parent
            ::n::Uuid_WithParameters, // UUID
            "::n::WithParameters", // Type name
            [](::Smp::String8 name,
                ::Smp::String8 description,
                ::Smp::IObject* parent,
                ::Smp::Publication::ITypeRegistry* type_registry) {
                return new ::n::WithParameters(name, description, parent, type_registry);
            }, // Instantiation callback
            typeRegistry // Type Registry
        ));

        // Register factory for Model WithReturn
        simulator->RegisterFactory(new ::TasMdk::Factory(
            "WithReturn", // Name
            "", // Description
            simulator, // parent
            ::n::Uuid_WithReturn, // UUID
            "::n::WithReturn", // Type name
            [](::Smp::String8 name,
                ::Smp::String8 description,
                ::Smp::IObject* parent,
                ::Smp::Publication::ITypeRegistry* type_registry) {
                return new ::n::WithReturn(name, description, parent, type_registry);
            }, // Instantiation callback
            typeRegistry // Type Registry
        ));

        // Register factory for Model Bare
        simulator->RegisterFactory(new ::TasMdk::Factory(
            "Bare", // Name
            "", // Description
            simulator, // parent
            ::n::Uuid_Bare, // UUID
            "::n::Bare", // Type name
            [](::Smp::String8 name,
                ::Smp::String8 description,
                ::Smp::IObject* parent,
                ::Smp::Publication::ITypeRegistry* type_registry) {
                return new ::n::Bare(name, description, parent, type_registry);
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
    /// Finalise Package Demo.
    /// @return True if finalisation was successful, false otherwise.
    #if ECSS_SMP_VERSION < 202503L
    bool Finalise_Demo() {
        // avoid double finalisation
        if (::simulators.empty()) {
            return true;
        }
        ::simulators.clear();

        return true;
    }
    #else
    bool Finalise_Demo(::Smp::ISimulator *simulator) {
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
