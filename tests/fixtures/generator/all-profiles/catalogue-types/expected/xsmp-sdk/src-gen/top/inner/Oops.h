// Copyright line one
// Copyright line two
// -----------------------------------------------------------------------------
// File Name    : Oops.h
// -----------------------------------------------------------------------------
/// @file top/inner/Oops.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef TOP_INNER_OOPS_H_
#define TOP_INNER_OOPS_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/ITypeRegistry.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

/// Inner namespace doc.
///  Second line.
namespace top::inner
{
class Oops: public ::Smp::Exception
{
    public:
    static void _Register(::Smp::Publication::ITypeRegistry* registry);

    Oops() = default;
    ~Oops() noexcept = default;
    Oops(const Oops&) = default;

    ::Smp::Int32 code;
};

/// Universally unique identifier of type Oops.
inline constexpr ::Smp::Uuid Uuid_Oops { 0x10000000U, 0x0000U, 0x4000U, 0x8000U, 0x00000000001bU };
} // namespace top::inner

#endif // TOP_INNER_OOPS_H_
