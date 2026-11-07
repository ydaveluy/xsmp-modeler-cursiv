// Copyright line one
// Copyright line two
// -----------------------------------------------------------------------------
// File Name    : Derived.h
// -----------------------------------------------------------------------------
/// @file top/inner/Derived.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef TOP_INNER_DERIVED_H_
#define TOP_INNER_DERIVED_H_

// ----------------------------------------------------------------------------
// --------------------------- Forward Declarations ---------------------------
// ----------------------------------------------------------------------------

namespace top::inner
{
class Derived;
} // namespace top::inner

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <top/inner/Base.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

/// Inner namespace doc.
///  Second line.
namespace top::inner
{
class Derived: public ::top::inner::Base
{
    public:
    static void _Register(::Smp::Publication::ITypeRegistry* registry);

    Derived() = default;
    ~Derived() noexcept = default;
    Derived(const Derived&) = default;

    Derived(::Smp::Int32 a);
    void operator+=(::Smp::Int32 a);
    ::Smp::Bool operator==(::top::inner::Derived other);
    ::Smp::Bool z;
};

/// Universally unique identifier of type Derived.
inline constexpr ::Smp::Uuid Uuid_Derived { 0x10000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000018U };
} // namespace top::inner

#endif // TOP_INNER_DERIVED_H_
