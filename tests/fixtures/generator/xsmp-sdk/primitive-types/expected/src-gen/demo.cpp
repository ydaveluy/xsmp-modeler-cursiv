// -----------------------------------------------------------------------------
// File Name    : demo.cpp
// -----------------------------------------------------------------------------
/// @file demo.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <demo.h>
#include <n/Any.h>
#include <n/Count.h>
#include <n/Foo.h>
#include <n/Hostile.h>
#include <n/Label.h>
#include <n/Narrow.h>
#include <n/Pct.h>
#include <n/Ratio.h>
#include <n/Small.h>
#include <n/Whole.h>
#include <n/Written.h>
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

        // register Integer Pct
        ::n::_Register_Pct(typeRegistry);
        // register Integer Any
        ::n::_Register_Any(typeRegistry);
        // register Integer Whole
        ::n::_Register_Whole(typeRegistry);
        // register Integer Written
        ::n::_Register_Written(typeRegistry);
        // register Integer Small
        ::n::_Register_Small(typeRegistry);
        // register Integer Narrow
        ::n::_Register_Narrow(typeRegistry);
        // register Integer Count
        ::n::_Register_Count(typeRegistry);
        // register Float Ratio
        ::n::_Register_Ratio(typeRegistry);
        // register Float Hostile
        ::n::_Register_Hostile(typeRegistry);
        // register String Label
        ::n::_Register_Label(typeRegistry);
        // register Integer Foo
        ::n::_Register_Foo(typeRegistry);

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
