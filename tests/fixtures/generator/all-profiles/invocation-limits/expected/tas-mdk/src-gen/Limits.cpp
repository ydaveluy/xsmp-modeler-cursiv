// -----------------------------------------------------------------------------
// File Name    : Limits.cpp
// -----------------------------------------------------------------------------
/// @file Limits.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <Limits.h>
#include <limits/Child.h>
#include <limits/OrdinaryClass.h>
#include <limits/Probe.h>
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
        simulator->RegisterFactory(new ::TasMdk::Factory(
            "Probe", // Name
            "", // Description
            simulator, // parent
            ::limits::Uuid_Probe, // UUID
            "::limits::Probe", // Type name
            [](::Smp::String8 name,
                ::Smp::String8 description,
                ::Smp::IObject* parent,
                ::Smp::Publication::ITypeRegistry* type_registry) {
                return new ::limits::Probe(name, description, parent, type_registry);
            }, // Instantiation callback
            typeRegistry // Type Registry
        ));

        // Register factory for Model Child
        simulator->RegisterFactory(new ::TasMdk::Factory(
            "Child", // Name
            "", // Description
            simulator, // parent
            ::limits::Uuid_Child, // UUID
            "::limits::Child", // Type name
            [](::Smp::String8 name,
                ::Smp::String8 description,
                ::Smp::IObject* parent,
                ::Smp::Publication::ITypeRegistry* type_registry) {
                return new ::limits::Child(name, description, parent, type_registry);
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
    /// Finalise Package Limits.
    /// @return True if finalisation was successful, false otherwise.
    #if ECSS_SMP_VERSION < 202503L
    bool Finalise_Limits() {
        // avoid double finalisation
        if (::simulators.empty()) {
            return true;
        }
        ::simulators.clear();

        return true;
    }
    #else
    bool Finalise_Limits(::Smp::ISimulator *simulator) {
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
