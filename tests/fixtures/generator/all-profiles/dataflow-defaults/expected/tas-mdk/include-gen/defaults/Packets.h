// -----------------------------------------------------------------------------
// File Name    : Packets.h
// -----------------------------------------------------------------------------
/// @file defaults/Packets.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef DEFAULTS_PACKETS_H_
#define DEFAULTS_PACKETS_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <defaults/Packet.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <TasMdk/Types/Array.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace defaults
{
using Packets = ::TasMdk::Types::Array<::defaults::Packet, 2>;

/// Universally unique identifier of type Packets.
constexpr ::Smp::Uuid Uuid_Packets { 0x70000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000008U };

void _Register_Packets(::Smp::Publication::ITypeRegistry* registry);

template<bool ...Opts>
using _Packets = TasMdk::Types::ArrayType<::defaults::_Packet, 2, Opts...>;
} // namespace defaults

#endif // DEFAULTS_PACKETS_H_
