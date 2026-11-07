// -----------------------------------------------------------------------------
// File Name    : ConstructedGen.h
// -----------------------------------------------------------------------------
/// @file n/ConstructedGen.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef N_CONSTRUCTEDGEN_H_
#define N_CONSTRUCTEDGEN_H_

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
class Constructed;

class ConstructedGen
{
    friend class ::n::Constructed;
    public:
    static void _Register(::Smp::Publication::ITypeRegistry* registry);

    ConstructedGen() = default;
    ~ConstructedGen() noexcept = default;
    ConstructedGen(const ConstructedGen&) = default;

    /// Get value.
    /// @return Current value of property value.
    ::Smp::Int32 get_value();
    /// Set value.
    /// @param value New value of property value to set.
    void set_value(::Smp::Int32 value);
    ConstructedGen(::Smp::Int32 a);
    ConstructedGen(::Smp::Float64 b);
};

/// Universally unique identifier of type Constructed.
constexpr ::Smp::Uuid Uuid_Constructed { 0x11111111U, 0x0000U, 0x4000U, 0x8000U, 0x000000000102U };
} // namespace n

#endif // N_CONSTRUCTEDGEN_H_
