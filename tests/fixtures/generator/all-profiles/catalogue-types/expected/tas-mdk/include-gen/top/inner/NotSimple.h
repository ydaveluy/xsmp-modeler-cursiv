// Copyright line one
// Copyright line two
// -----------------------------------------------------------------------------
// File Name    : NotSimple.h
// -----------------------------------------------------------------------------
/// @file top/inner/NotSimple.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef TOP_INNER_NOTSIMPLE_H_
#define TOP_INNER_NOTSIMPLE_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <TasMdk/Types/Array.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

/// Top namespace doc.
namespace top
{
/// Inner namespace doc.
///  Second line.
namespace inner
{
using NotSimple = ::TasMdk::Types::Array<::Smp::Float64, 2>;

/// Universally unique identifier of type NotSimple.
constexpr ::Smp::Uuid Uuid_NotSimple { 0x10000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000010U };

void _Register_NotSimple(::Smp::Publication::ITypeRegistry* registry);

template<bool ...Opts>
using _NotSimple = TasMdk::Types::ArrayType<::Smp::Float64, 2, Opts...>;
} // namespace inner
} // namespace top

#endif // TOP_INNER_NOTSIMPLE_H_
