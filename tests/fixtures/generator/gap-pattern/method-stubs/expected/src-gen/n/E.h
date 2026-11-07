// -----------------------------------------------------------------------------
// File Name    : E.h
// -----------------------------------------------------------------------------
/// @file n/E.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef N_E_H_
#define N_E_H_

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
enum class E: ::Smp::Int32 {
    A = 1
};

/// Universally unique identifier of type E.
inline constexpr ::Smp::Uuid Uuid_E { 0x11111111U, 0x0000U, 0x4000U, 0x8000U, 0x0000000000f2U };

void _Register_E(::Smp::Publication::ITypeRegistry* registry);

const std::map<E, std::string> E_name_map = {
    { E::A, "A" }
};

const std::map<E, std::string> E_descr_map = {
    { E::A, "" }
};
} // namespace n

#endif // N_E_H_
