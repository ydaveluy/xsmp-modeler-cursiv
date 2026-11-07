// -----------------------------------------------------------------------------
// File Name    : DataflowDefaults.cpp
// -----------------------------------------------------------------------------
/// @file DataflowDefaults.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <DataflowDefaults.h>
#include <defaults/Cube.h>
#include <defaults/Hypercube.h>
#include <defaults/Matrix.h>
#include <defaults/Packet.h>
#include <defaults/Packets.h>
#include <defaults/Point.h>
#include <defaults/Points.h>
#include <defaults/Probe.h>
#include <defaults/Row.h>
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
    /// Initialise Package DataflowDefaults.
    /// @param simulator Simulator for registration of factories.
    /// @param typeRegistry Type Registry for registration of types.
    /// @return True if initialisation was successful, false otherwise.
    bool Initialise_DataflowDefaults(
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

        // register Array Row
        ::defaults::_Register_Row(typeRegistry);
        // register Structure Point
        ::defaults::Point::_Register(typeRegistry);
        // register Array Matrix
        ::defaults::_Register_Matrix(typeRegistry);
        // register Structure Packet
        ::defaults::Packet::_Register(typeRegistry);
        // register Array Points
        ::defaults::_Register_Points(typeRegistry);
        // register Array Cube
        ::defaults::_Register_Cube(typeRegistry);
        // register Array Packets
        ::defaults::_Register_Packets(typeRegistry);
        // register Array Hypercube
        ::defaults::_Register_Hypercube(typeRegistry);

        // Register factory for Model Probe
        simulator->RegisterFactory(::Xsmp::Factory::Create<::defaults::Probe>(
            "Probe", // Name
            "", // Description
            simulator, // Simulator
            ::defaults::Uuid_Probe // UUID
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
    /// Finalise Package DataflowDefaults.
    /// @param simulator Simulator to finalise.
    /// @return True if finalisation was successful, false otherwise.
    bool Finalise_DataflowDefaults(::Smp::ISimulator* simulator) {
        // avoid double finalisation of that simulator
        if (::simulators.erase(simulator) == 0) {
            return true;
        }
        return true;
    }

    /// Get the revision of the SMP standard Package DataflowDefaults was
    /// built against.
    /// @return The revision of the SMP standard.
    ::Smp::UInt64 GetSmpVersion_DataflowDefaults() {
        return ECSS_SMP_VERSION;
    }
#else
    /// Finalise Package DataflowDefaults.
    /// @return True if finalisation was successful, false otherwise.
    bool Finalise_DataflowDefaults() {
        // avoid double finalisation
        if (::simulators.empty()) {
            return true;
        }
        ::simulators.clear();

        return true;
    }
#endif
}
