// Copyright line one
// Copyright line two
// -----------------------------------------------------------------------------
// File Name    : Empty.h
// -----------------------------------------------------------------------------
/// @file top/inner/Empty.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef TOP_INNER_EMPTY_H_
#define TOP_INNER_EMPTY_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <Smp/Publication/ITypeRegistry.h>
#include <Smp/ViewKind.h>

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
struct Empty
{
    static void _Register(::Smp::Publication::ITypeRegistry* registry);
};

/// Universally unique identifier of type Empty.
constexpr ::Smp::Uuid Uuid_Empty { 0x10000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000016U };
} // namespace inner
} // namespace top

#endif // TOP_INNER_EMPTY_H_
