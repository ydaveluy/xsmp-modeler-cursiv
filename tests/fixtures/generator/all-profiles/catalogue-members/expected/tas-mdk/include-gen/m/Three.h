// Block notice
// line two
// -----------------------------------------------------------------------------
// File Name    : Three.h
// -----------------------------------------------------------------------------
/// @file m/Three.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef M_THREE_H_
#define M_THREE_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <TasMdk/Types/Array.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace m
{
using Three = ::TasMdk::Types::Array<::Smp::Int32, 3>;

/// Universally unique identifier of type Three.
constexpr ::Smp::Uuid Uuid_Three { 0x20000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000003U };

void _Register_Three(::Smp::Publication::ITypeRegistry* registry);

template<bool ...Opts>
using _Three = TasMdk::Types::SimpleArrayType<::Smp::Int32, 3, Opts...>;
} // namespace m

#endif // M_THREE_H_
