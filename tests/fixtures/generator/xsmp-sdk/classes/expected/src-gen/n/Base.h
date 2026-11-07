// -----------------------------------------------------------------------------
// File Name    : Base.h
// -----------------------------------------------------------------------------
/// @file n/Base.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef N_BASE_H_
#define N_BASE_H_

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
class Base
{
    public:
    static void _Register(::Smp::Publication::ITypeRegistry* registry);

    Base() = default;
    ~Base() noexcept = default;
    Base(const Base&) = default;

    private:
    static constexpr ::Smp::Int32 K{7};
    void doIt();
    ::Smp::Int32 x;
};

/// Universally unique identifier of type Base.
inline constexpr ::Smp::Uuid Uuid_Base { 0x11111111U, 0x0000U, 0x4000U, 0x8000U, 0x000000000041U };
} // namespace n

#endif // N_BASE_H_
