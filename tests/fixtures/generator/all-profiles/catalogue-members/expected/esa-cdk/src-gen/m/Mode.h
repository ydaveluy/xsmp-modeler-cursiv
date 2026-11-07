// Block notice
// line two
// -----------------------------------------------------------------------------
// File Name    : Mode.h
// -----------------------------------------------------------------------------
/// @file m/Mode.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef M_MODE_H_
#define M_MODE_H_

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

namespace m
{
enum class Mode: ::Smp::Int32 {
    Off = 0,
    On = 1
};

/// Universally unique identifier of type Mode.
constexpr ::Smp::Uuid Uuid_Mode { 0x20000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000001U };

void _Register_Mode(::Smp::Publication::ITypeRegistry* registry);

const std::map<Mode, std::string> Mode_name_map = {
    { Mode::Off, "Off" }, { Mode::On, "On" }
};

const std::map<Mode, std::string> Mode_descr_map = {
    { Mode::Off, "" }, { Mode::On, "" }
};
} // namespace m

#endif // M_MODE_H_
