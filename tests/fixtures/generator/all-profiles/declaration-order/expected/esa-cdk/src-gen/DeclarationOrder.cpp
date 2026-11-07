// -----------------------------------------------------------------------------
// File Name    : DeclarationOrder.cpp
// -----------------------------------------------------------------------------
/// @file DeclarationOrder.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <DeclarationOrder.h>
#include <order/Child.h>
#include <order/Holder.h>
#include <order/Inner.h>
#include <order/Outer.h>
#include <order/Parent.h>
#include <order/Value.h>
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
    /// Initialise Package DeclarationOrder.
    /// @param simulator Simulator for registration of factories.
    /// @param typeRegistry Type Registry for registration of types.
    /// @return True if initialisation was successful, false otherwise.
    bool Initialise_DeclarationOrder(
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

        // register Structure Value
        ::order::Value::_Register(typeRegistry);
        // register Class Parent
        ::order::Parent::_Register(typeRegistry);
        // register Array Inner
        ::order::_Register_Inner(typeRegistry);
        // register Structure Holder
        ::order::Holder::_Register(typeRegistry);
        // register Class Child
        ::order::Child::_Register(typeRegistry);
        // register Array Outer
        ::order::_Register_Outer(typeRegistry);

        return true;
    }
}

// ---------------------------------------------------------------------------------
// ---------------------------- Finalise Function ------------------------------
// ---------------------------------------------------------------------------------

extern "C"
{
#if ECSS_SMP_VERSION >= 202503L
    /// Finalise Package DeclarationOrder.
    /// @param simulator Simulator to finalise.
    /// @return True if finalisation was successful, false otherwise.
    bool Finalise_DeclarationOrder(::Smp::ISimulator* simulator) {
        // avoid double finalisation of that simulator
        if (::simulators.erase(simulator) == 0) {
            return true;
        }
        return true;
    }

    /// Get the revision of the SMP standard Package DeclarationOrder was
    /// built against.
    /// @return The revision of the SMP standard.
    ::Smp::UInt64 GetSmpVersion_DeclarationOrder() {
        return ECSS_SMP_VERSION;
    }
#else
    /// Finalise Package DeclarationOrder.
    /// @return True if finalisation was successful, false otherwise.
    bool Finalise_DeclarationOrder() {
        // avoid double finalisation
        if (::simulators.empty()) {
            return true;
        }
        ::simulators.clear();

        return true;
    }
#endif
}
