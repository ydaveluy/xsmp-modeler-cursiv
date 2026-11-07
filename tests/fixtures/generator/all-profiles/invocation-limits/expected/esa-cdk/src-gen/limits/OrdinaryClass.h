// -----------------------------------------------------------------------------
// File Name    : OrdinaryClass.h
// -----------------------------------------------------------------------------
/// @file limits/OrdinaryClass.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef LIMITS_ORDINARYCLASS_H_
#define LIMITS_ORDINARYCLASS_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/ITypeRegistry.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace limits
{
class OrdinaryClass
{
    public:
    static void _Register(::Smp::Publication::ITypeRegistry* registry);

    OrdinaryClass() = default;
    ~OrdinaryClass() noexcept = default;
    OrdinaryClass(const OrdinaryClass&) = default;

    private:
    OrdinaryClass(::Smp::Int32 value);
    ::Smp::Int32* pointer();
    ::Smp::Int32 calculate(::Smp::Int32 value);
    ::Smp::Int32 calculate(::Smp::Float64 value);
};

/// Universally unique identifier of type OrdinaryClass.
constexpr ::Smp::Uuid Uuid_OrdinaryClass { 0x87000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000003U };
} // namespace limits

#endif // LIMITS_ORDINARYCLASS_H_
