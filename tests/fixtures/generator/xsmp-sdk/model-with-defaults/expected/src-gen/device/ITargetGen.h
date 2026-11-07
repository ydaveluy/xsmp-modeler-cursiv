// -----------------------------------------------------------------------------
// File Name    : ITargetGen.h
// -----------------------------------------------------------------------------
/// @file device/ITargetGen.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef DEVICE_ITARGETGEN_H_
#define DEVICE_ITARGETGEN_H_

// ----------------------------------------------------------------------------
// --------------------------- Forward Declarations ---------------------------
// ----------------------------------------------------------------------------

namespace device
{
class ITarget;
} // namespace device

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <Smp/Uuid.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace device
{
class ITargetGen {
public:
    virtual ~ITargetGen() = default;
};

/// Universally unique identifier of type ITarget.
inline constexpr ::Smp::Uuid Uuid_ITarget { 0x44444444U, 0x4444U, 0x4444U, 0x8444U, 0x444444444444U };
} // namespace device

#endif // DEVICE_ITARGETGEN_H_
