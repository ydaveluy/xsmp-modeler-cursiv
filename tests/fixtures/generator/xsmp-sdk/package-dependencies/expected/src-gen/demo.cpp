// A field of `other` and an operation of `types` chain their packages. An
// interface has no implementation, so that `interfaces` is not chained.
// -----------------------------------------------------------------------------
// File Name    : demo.cpp
// -----------------------------------------------------------------------------
/// @file demo.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <demo.h>
#include <n2/M.h>
#include <n2/S.h>
#include <other.h>
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

        // Initialisation of dependent Package(s)
        if(!Initialise_other(simulator, typeRegistry)) {
            return false;
        }

        // register Structure S
        ::n2::S::_Register(typeRegistry);

        // Register factory for Model M
        simulator->RegisterFactory(::Xsmp::Factory::Create<::n2::M>(
            "M", // Name
            "", // Description
            simulator, // Simulator
            ::n2::Uuid_M // UUID
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
        // Finalisation of dependent Package(s)
        return Finalise_other(simulator);
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

        // Finalisation of dependent Package(s)
        return Finalise_other();
    }
#endif
}
