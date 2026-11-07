// Copyright line one
// Copyright line two
// -----------------------------------------------------------------------------
// File Name    : Later2.h
// -----------------------------------------------------------------------------
/// @file top/other/Later2.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef TOP_OTHER_LATER2_H_
#define TOP_OTHER_LATER2_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <esa/ecss/smp/cdk/Array.h>
#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/ITypeRegistry.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

/// Top namespace doc.
namespace top
{
namespace other
{
using Later2 = ::esa::ecss::smp::cdk::Array<::Smp::Int32, 1>;

/// Universally unique identifier of type Later2.
constexpr ::Smp::Uuid Uuid_Later2 { 0x10000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000023U };

void _Register_Later2(::Smp::Publication::ITypeRegistry* registry);
} // namespace other
} // namespace top

#endif // TOP_OTHER_LATER2_H_
