// -----------------------------------------------------------------------------
// File Name    : A.h
// -----------------------------------------------------------------------------
/// @file dependencies/A.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef DEPENDENCIES_A_H_
#define DEPENDENCIES_A_H_

// ----------------------------------------------------------------------------
// --------------------------- Forward Declarations ---------------------------
// ----------------------------------------------------------------------------

namespace dependencies
{
class B;
} // namespace dependencies

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <dependencies/B.h>
#include <Smp/Publication/ITypeRegistry.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace dependencies
{
class A
{
    public:
    static void _Register(::Smp::Publication::ITypeRegistry* registry);

    A() = default;
    ~A() noexcept = default;
    A(const A&) = default;

    private:
    void byValue(::dependencies::B other);
    void byReference(::dependencies::B& other);
    void byPointer(::dependencies::B* other);
};

/// Universally unique identifier of type A.
constexpr ::Smp::Uuid Uuid_A { 0x82000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000003U };
} // namespace dependencies

#endif // DEPENDENCIES_A_H_
