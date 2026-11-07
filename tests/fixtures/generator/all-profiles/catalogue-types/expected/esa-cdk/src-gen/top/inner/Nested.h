// Copyright line one
// Copyright line two
// -----------------------------------------------------------------------------
// File Name    : Nested.h
// -----------------------------------------------------------------------------
/// @file top/inner/Nested.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef TOP_INNER_NESTED_H_
#define TOP_INNER_NESTED_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <Smp/ViewKind.h>
#include <top/inner/Payload.h>

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
struct Nested
{
    ::top::inner::Payload inner;
    ::Smp::Bool ok;

    static void _Register(::Smp::Publication::ITypeRegistry* registry);
};

/// Universally unique identifier of type Nested.
constexpr ::Smp::Uuid Uuid_Nested { 0x10000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000015U };
} // namespace inner
} // namespace top

#endif // TOP_INNER_NESTED_H_
