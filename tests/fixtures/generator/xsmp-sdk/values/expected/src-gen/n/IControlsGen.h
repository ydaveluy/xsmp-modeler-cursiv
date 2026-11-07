// -----------------------------------------------------------------------------
// File Name    : IControlsGen.h
// -----------------------------------------------------------------------------
/// @file n/IControlsGen.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef N_ICONTROLSGEN_H_
#define N_ICONTROLSGEN_H_

// ----------------------------------------------------------------------------
// --------------------------- Forward Declarations ---------------------------
// ----------------------------------------------------------------------------

namespace n
{
class IControls;
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
class IControlsGen {
public:
    virtual ~IControlsGen() = default;
    virtual void f(::Smp::String8 a = "a\0b")=0;
};

/// Universally unique identifier of type IControls.
inline constexpr ::Smp::Uuid Uuid_IControls { 0x11111111U, 0x0000U, 0x4000U, 0x8000U, 0x000000000096U };
} // namespace n

#endif // N_ICONTROLSGEN_H_
