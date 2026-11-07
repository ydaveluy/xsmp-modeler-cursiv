// -----------------------------------------------------------------------------
// File Name    : Packet.h
// -----------------------------------------------------------------------------
/// @file defaults/Packet.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef DEFAULTS_PACKET_H_
#define DEFAULTS_PACKET_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <defaults/Row.h>
#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <Smp/ViewKind.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace defaults
{
struct Packet
{
    ::defaults::Row samples;
    ::Smp::Int32 tag;

    static void _Register(::Smp::Publication::ITypeRegistry* registry);
};

/// Universally unique identifier of type Packet.
constexpr ::Smp::Uuid Uuid_Packet { 0x70000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000007U };
} // namespace defaults

#endif // DEFAULTS_PACKET_H_
