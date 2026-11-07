// Copyright line one
// Copyright line two
// -----------------------------------------------------------------------------
// File Name    : Pure.h
// -----------------------------------------------------------------------------
/// @file top/inner/Pure.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef TOP_INNER_PURE_H_
#define TOP_INNER_PURE_H_

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
/// A class that cannot be instantiated.
class Pure
{
    public:
    static void _Register(::Smp::Publication::ITypeRegistry* registry);

    Pure() = default;
    ~Pure() noexcept = default;
    Pure(const Pure&) = default;

    virtual ::Smp::Int32 pure()=0;
};

/// Universally unique identifier of type Pure.
inline constexpr ::Smp::Uuid Uuid_Pure { 0x10000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000026U };
} // namespace top::inner

#endif // TOP_INNER_PURE_H_
