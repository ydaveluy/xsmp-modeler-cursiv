// -----------------------------------------------------------------------------
// File Name    : Foo.h
// -----------------------------------------------------------------------------
/// @file n/Foo.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef N_FOO_H_
#define N_FOO_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/ITypeRegistry.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace n
{
/// Universally unique identifier of type Foo.
inline constexpr ::Smp::Uuid Uuid_Foo { 0x11111111U, 0x0000U, 0x4000U, 0x8000U, 0x00000000001aU };
using Foo = ::Smp::Int32;
void _Register_Foo(::Smp::Publication::ITypeRegistry* registry);
} // namespace n

#endif // N_FOO_H_
