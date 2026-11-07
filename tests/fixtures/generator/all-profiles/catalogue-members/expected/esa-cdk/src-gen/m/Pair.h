// Block notice
// line two
// -----------------------------------------------------------------------------
// File Name    : Pair.h
// -----------------------------------------------------------------------------
/// @file m/Pair.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef M_PAIR_H_
#define M_PAIR_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <Smp/ViewKind.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace m
{
struct Pair
{
    ::Smp::Int32 a{1};
    ::Smp::Int32 b;

    Pair() = default;
    ~Pair() = default;
    Pair(const Pair&) = default;
    Pair(Pair&&) = default;
    Pair(::Smp::Int32 a, ::Smp::Int32 b):
    a(a), b(b) {}
    Pair& operator=(const Pair&) = default;

    static void _Register(::Smp::Publication::ITypeRegistry* registry);
};

/// Universally unique identifier of type Pair.
constexpr ::Smp::Uuid Uuid_Pair { 0x20000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000002U };
} // namespace m

#endif // M_PAIR_H_
