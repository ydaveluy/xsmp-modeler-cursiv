// -----------------------------------------------------------------------------
// File Name    : Demo.cpp
// -----------------------------------------------------------------------------
/// @file Demo.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <Demo.h>
#include <n/C.h>
#include <n/ConstProperty.h>
#include <n/Constructed.h>
#include <n/M.h>
#include <n/Text.h>
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

        // register String Text
        ::n::_Register_Text(typeRegistry);
        // register Class C
        ::n::C::_Register(typeRegistry);

        // Register factory for Model M
        simulator->RegisterFactory(::Xsmp::Factory::Create<::n::M>(
            "M", // Name
            "", // Description
            simulator, // Simulator
            ::n::Uuid_M // UUID
        ));

        // Register factory for Model Constructed
        simulator->RegisterFactory(::Xsmp::Factory::Create<::n::Constructed>(
            "Constructed", // Name
            "", // Description
            simulator, // Simulator
            ::n::Uuid_Constructed // UUID
        ));

        // Register factory for Model ConstProperty
        simulator->RegisterFactory(::Xsmp::Factory::Create<::n::ConstProperty>(
            "ConstProperty", // Name
            "", // Description
            simulator, // Simulator
            ::n::Uuid_ConstProperty // UUID
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
