// -----------------------------------------------------------------------------
// File Name    : Cube.h
// -----------------------------------------------------------------------------
/// @file defaults/Cube.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef DEFAULTS_CUBE_H_
#define DEFAULTS_CUBE_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <defaults/Matrix.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <TasMdk/Types/Array.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace defaults
{
using Cube = ::TasMdk::Types::Array<::defaults::Matrix, 2>;

/// Universally unique identifier of type Cube.
constexpr ::Smp::Uuid Uuid_Cube { 0x70000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000003U };

void _Register_Cube(::Smp::Publication::ITypeRegistry* registry);

template<bool ...Opts>
using _Cube = TasMdk::Types::ArrayType<::defaults::_Matrix, 2, Opts...>;
} // namespace defaults

#endif // DEFAULTS_CUBE_H_
