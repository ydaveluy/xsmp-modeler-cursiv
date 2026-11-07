// Block notice
// line two
// -----------------------------------------------------------------------------
// File Name    : IBaseGen.h
// -----------------------------------------------------------------------------
/// @file m/IBaseGen.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef M_IBASEGEN_H_
#define M_IBASEGEN_H_

// ----------------------------------------------------------------------------
// --------------------------- Forward Declarations ---------------------------
// ----------------------------------------------------------------------------

namespace m
{
class IBase;
} // namespace m

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <m/Name.h>
#include <Smp/PrimitiveTypes.h>
#include <Smp/Uuid.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace m
{
/// A device interface.
class IBaseGen {
public:
    virtual ~IBaseGen() = default;
    static constexpr ::Smp::Bool Ready{false};
    static const ::m::Name Label;
    /// Get connected.
    /// @return Current value of property connected.
    virtual ::Smp::Bool get_connected()=0;
    /// Set connected.
    /// @param value New value of property connected to set.
    virtual void set_connected(::Smp::Bool value)=0;
    /// Get ro.
    /// @return Current value of property ro.
    virtual ::Smp::Int32 get_ro()=0;
    /// Set wo.
    /// @param value New value of property wo to set.
    virtual void set_wo(::Smp::Int32 value)=0;
    /// Ping it.
    /// @param value The value.
    /// @return The answer.
    virtual ::Smp::Bool ping(::Smp::Int32 value)=0;
};

/// Universally unique identifier of type IBase.
inline constexpr ::Smp::Uuid Uuid_IBase { 0x20000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000010U };
} // namespace m

#endif // M_IBASEGEN_H_
