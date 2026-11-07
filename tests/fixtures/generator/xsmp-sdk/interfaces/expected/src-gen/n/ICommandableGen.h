// -----------------------------------------------------------------------------
// File Name    : ICommandableGen.h
// -----------------------------------------------------------------------------
/// @file n/ICommandableGen.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef N_ICOMMANDABLEGEN_H_
#define N_ICOMMANDABLEGEN_H_

// ----------------------------------------------------------------------------
// --------------------------- Forward Declarations ---------------------------
// ----------------------------------------------------------------------------

namespace n
{
class ICommandable;
} // namespace n

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <Smp/PrimitiveTypes.h>
#include <Smp/Uuid.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace n
{
class ICommandableGen {
public:
    virtual ~ICommandableGen() = default;
    /// Get Mode.
    /// @return Current value of property Mode.
    virtual ::Smp::Int32 get_Mode()=0;
    /// Set Mode.
    /// @param value New value of property Mode to set.
    virtual void set_Mode(::Smp::Int32 value)=0;
    virtual void apply(::Smp::Int32 requestedMode)=0;
};

/// Universally unique identifier of type ICommandable.
inline constexpr ::Smp::Uuid Uuid_ICommandable { 0x11111111U, 0x0000U, 0x4000U, 0x8000U, 0x000000000051U };
} // namespace n

#endif // N_ICOMMANDABLEGEN_H_
