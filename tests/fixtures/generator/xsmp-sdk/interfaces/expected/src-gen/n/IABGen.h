// -----------------------------------------------------------------------------
// File Name    : IABGen.h
// -----------------------------------------------------------------------------
/// @file n/IABGen.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef N_IABGEN_H_
#define N_IABGEN_H_

// ----------------------------------------------------------------------------
// --------------------------- Forward Declarations ---------------------------
// ----------------------------------------------------------------------------

namespace n
{
class IAB;
} // namespace n

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <n/IA.h>
#include <n/IB.h>
#include <Smp/Uuid.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace n
{
class IABGen: public virtual ::n::IA, public virtual ::n::IB {
public:
    virtual ~IABGen() = default;
};

/// Universally unique identifier of type IAB.
inline constexpr ::Smp::Uuid Uuid_IAB { 0x11111111U, 0x0000U, 0x4000U, 0x8000U, 0x000000000054U };
} // namespace n

#endif // N_IABGEN_H_
