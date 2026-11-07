// -----------------------------------------------------------------------------
// File Name    : Mode.h
// -----------------------------------------------------------------------------
/// @file n/Mode.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef N_MODE_H_
#define N_MODE_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <map>
#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <string>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace n
{
enum class Mode: ::Smp::Int32 {
    /// Idle mode.
    Standby = 0,
    Nominal = 1,
    /// Component is powered off and fully idle.
    Off = 2
};

/// Universally unique identifier of type Mode.
inline constexpr ::Smp::Uuid Uuid_Mode { 0x11111111U, 0x0000U, 0x4000U, 0x8000U, 0x000000000021U };

void _Register_Mode(::Smp::Publication::ITypeRegistry* registry);

const std::map<Mode, std::string> Mode_name_map = {
    { Mode::Standby, "Standby" }, { Mode::Nominal, "Nominal" }, { Mode::Off, "Off" }
};

const std::map<Mode, std::string> Mode_descr_map = {
    { Mode::Standby, "Idle mode." }, { Mode::Nominal, "" }, { Mode::Off, "Component is powered off and fully idle." }
};
} // namespace n

#endif // N_MODE_H_
