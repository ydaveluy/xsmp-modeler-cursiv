// Block notice
// line two
// -----------------------------------------------------------------------------
// File Name    : Name.h
// -----------------------------------------------------------------------------
/// @file m/Name.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef M_NAME_H_
#define M_NAME_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <esa/ecss/smp/cdk/String.h>
#include <Smp/Publication/ITypeRegistry.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace m
{
using Name = ::esa::ecss::smp::cdk::String<8>;

/// Universally unique identifier of type Name.
constexpr ::Smp::Uuid Uuid_Name { 0x20000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000004U };

void _Register_Name(::Smp::Publication::ITypeRegistry* registry);
} // namespace m

#endif // M_NAME_H_
