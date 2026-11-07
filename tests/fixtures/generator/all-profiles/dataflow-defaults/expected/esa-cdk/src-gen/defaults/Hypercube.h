// -----------------------------------------------------------------------------
// File Name    : Hypercube.h
// -----------------------------------------------------------------------------
/// @file defaults/Hypercube.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef DEFAULTS_HYPERCUBE_H_
#define DEFAULTS_HYPERCUBE_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <defaults/Cube.h>
#include <esa/ecss/smp/cdk/Array.h>
#include <Smp/Publication/ITypeRegistry.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace defaults
{
using Hypercube = ::esa::ecss::smp::cdk::Array<::defaults::Cube, 2>;

/// Universally unique identifier of type Hypercube.
constexpr ::Smp::Uuid Uuid_Hypercube { 0x70000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000004U };

void _Register_Hypercube(::Smp::Publication::ITypeRegistry* registry);
} // namespace defaults

#endif // DEFAULTS_HYPERCUBE_H_
