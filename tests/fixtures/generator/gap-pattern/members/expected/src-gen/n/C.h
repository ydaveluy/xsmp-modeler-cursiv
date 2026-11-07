// -----------------------------------------------------------------------------
// File Name    : C.h
// -----------------------------------------------------------------------------
/// @file n/C.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef N_C_H_
#define N_C_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/ITypeRegistry.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace n
{
class C
{
    public:
    static void _Register(::Smp::Publication::ITypeRegistry* registry);

    C() = default;
    ~C() noexcept = default;
    C(const C&) = default;

    /// Get value.
    /// @return Current value of property value.
    ::Smp::Int32 get_value();
    /// Set value.
    /// @param value New value of property value to set.
    void set_value(::Smp::Int32 value);
    C(::Smp::Int32 a);
    static ::Smp::Int32 shared;
};

/// Universally unique identifier of type C.
inline constexpr ::Smp::Uuid Uuid_C { 0x11111111U, 0x0000U, 0x4000U, 0x8000U, 0x0000000000b7U };
} // namespace n

#endif // N_C_H_
