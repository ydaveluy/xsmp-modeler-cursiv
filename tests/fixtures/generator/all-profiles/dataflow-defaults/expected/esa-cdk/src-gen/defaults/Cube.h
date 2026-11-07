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
#include <esa/ecss/smp/cdk/Array.h>
#include <Smp/Publication/ITypeRegistry.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace defaults
{
using Cube = ::esa::ecss::smp::cdk::Array<::defaults::Matrix, 2>;

/// Universally unique identifier of type Cube.
constexpr ::Smp::Uuid Uuid_Cube { 0x70000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000003U };

void _Register_Cube(::Smp::Publication::ITypeRegistry* registry);
} // namespace defaults

#endif // DEFAULTS_CUBE_H_
