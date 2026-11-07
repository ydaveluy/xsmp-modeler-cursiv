// -----------------------------------------------------------------------------
// File Name    : IGen.h
// -----------------------------------------------------------------------------
/// @file n/IGen.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef N_IGEN_H_
#define N_IGEN_H_

// ----------------------------------------------------------------------------
// --------------------------- Forward Declarations ---------------------------
// ----------------------------------------------------------------------------

namespace n
{
class M;
} // namespace n

namespace n
{
class I;
} // namespace n

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <Smp/Uuid.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace n
{
class IGen {
public:
    virtual ~IGen() = default;
    virtual void attach(const ::n::M& m)=0;
};

/// Universally unique identifier of type I.
inline constexpr ::Smp::Uuid Uuid_I { 0x11111111U, 0x0000U, 0x4000U, 0x8000U, 0x0000000000e1U };
} // namespace n

#endif // N_IGEN_H_
