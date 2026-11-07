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

/// Top namespace doc.
namespace top
{
namespace inner
{
class Derived;
} // namespace inner
} // namespace top

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <top/inner/Base.h>

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
constexpr ::Smp::Uuid Uuid_Derived { 0x10000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000018U };
} // namespace inner
} // namespace top

#endif // TOP_INNER_DERIVED_H_
