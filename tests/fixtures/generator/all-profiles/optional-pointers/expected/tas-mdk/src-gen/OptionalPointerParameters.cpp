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
    /// Initialise Package OptionalPointerParameters.
    /// @param simulator Simulator for registration of factories.
    /// @param typeRegistry Type Registry for registration of types.
    /// @return True if initialisation was successful, false otherwise.
    bool Initialise_OptionalPointerParameters(
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

        // Register factory for Model Probe
        simulator->RegisterFactory(new ::TasMdk::Factory(
            "Probe", // Name
            "", // Description
            simulator, // parent
            ::optional::Uuid_Probe, // UUID
            "::optional::Probe", // Type name
            [](::Smp::String8 name,
                ::Smp::String8 description,
                ::Smp::IObject* parent,
                ::Smp::Publication::ITypeRegistry* type_registry) {
                return new ::optional::Probe(name, description, parent, type_registry);
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
    /// Finalise Package OptionalPointerParameters.
    /// @return True if finalisation was successful, false otherwise.
    #if ECSS_SMP_VERSION < 202503L
    bool Finalise_OptionalPointerParameters() {
        // avoid double finalisation
        if (::simulators.empty()) {
            return true;
        }
        ::simulators.clear();

        return true;
    }
    #else
    bool Finalise_OptionalPointerParameters(::Smp::ISimulator *simulator) {
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
