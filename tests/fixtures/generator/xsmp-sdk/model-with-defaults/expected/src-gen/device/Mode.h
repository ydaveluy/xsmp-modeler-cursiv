// -----------------------------------------------------------------------------
// File Name    : Mode.h
// -----------------------------------------------------------------------------
/// @file device/Mode.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef DEVICE_MODE_H_
#define DEVICE_MODE_H_

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

namespace device
{
enum class Mode: ::Smp::Int32 {
    Off = 0,
    On = 1
};

/// Universally unique identifier of type Mode.
inline constexpr ::Smp::Uuid Uuid_Mode { 0x11111111U, 0x1111U, 0x4111U, 0x8111U, 0x111111111111U };

void _Register_Mode(::Smp::Publication::ITypeRegistry* registry);

const std::map<Mode, std::string> Mode_name_map = {
    { Mode::Off, "Off" }, { Mode::On, "On" }
};

const std::map<Mode, std::string> Mode_descr_map = {
    { Mode::Off, "" }, { Mode::On, "" }
};
} // namespace device

#endif // DEVICE_MODE_H_
