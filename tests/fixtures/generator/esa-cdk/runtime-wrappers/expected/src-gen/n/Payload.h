// -----------------------------------------------------------------------------
// File Name    : Payload.h
// -----------------------------------------------------------------------------
/// @file n/Payload.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef N_PAYLOAD_H_
#define N_PAYLOAD_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <Smp/ViewKind.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace n
{
struct Payload
{
    ::Smp::Bool enabled{false};

    Payload() = default;
    ~Payload() = default;
    Payload(const Payload&) = default;
    Payload(Payload&&) = default;
    Payload(::Smp::Bool enabled):
    enabled(enabled) {}
    Payload& operator=(const Payload&) = default;

    static void _Register(::Smp::Publication::ITypeRegistry* registry);
};

/// Universally unique identifier of type Payload.
constexpr ::Smp::Uuid Uuid_Payload { 0x11111111U, 0x0000U, 0x4000U, 0x8000U, 0x000000000113U };
} // namespace n

#endif // N_PAYLOAD_H_
