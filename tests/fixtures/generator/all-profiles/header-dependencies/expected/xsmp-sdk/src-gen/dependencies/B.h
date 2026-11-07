// -----------------------------------------------------------------------------
// File Name    : B.h
// -----------------------------------------------------------------------------
/// @file dependencies/B.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef DEPENDENCIES_B_H_
#define DEPENDENCIES_B_H_

// ----------------------------------------------------------------------------
// --------------------------- Forward Declarations ---------------------------
// ----------------------------------------------------------------------------

namespace dependencies
{
class A;
} // namespace dependencies

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <dependencies/A.h>
#include <Smp/Publication/ITypeRegistry.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace dependencies
{
class B
{
    public:
    static void _Register(::Smp::Publication::ITypeRegistry* registry);

    B() = default;
    ~B() noexcept = default;
    B(const B&) = default;

    private:
    void byValue(::dependencies::A other);
    void byReference(::dependencies::A& other);
    void byPointer(::dependencies::A* other);
};

/// Universally unique identifier of type B.
inline constexpr ::Smp::Uuid Uuid_B { 0x82000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000004U };
} // namespace dependencies

#endif // DEPENDENCIES_B_H_
