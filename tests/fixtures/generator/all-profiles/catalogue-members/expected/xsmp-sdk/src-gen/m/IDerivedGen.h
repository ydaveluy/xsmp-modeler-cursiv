// Block notice
// line two
// -----------------------------------------------------------------------------
// File Name    : IDerivedGen.h
// -----------------------------------------------------------------------------
/// @file m/IDerivedGen.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef M_IDERIVEDGEN_H_
#define M_IDERIVEDGEN_H_

// ----------------------------------------------------------------------------
// --------------------------- Forward Declarations ---------------------------
// ----------------------------------------------------------------------------

namespace m
{
class IDerived;
} // namespace m

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <m/IBase.h>
#include <Smp/IComponent.h>
#include <Smp/Uuid.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace m
{
class IDerivedGen: public virtual ::m::IBase, public virtual ::Smp::IComponent {
public:
    virtual ~IDerivedGen() = default;
    virtual void other()=0;
};

/// Universally unique identifier of type IDerived.
inline constexpr ::Smp::Uuid Uuid_IDerived { 0x20000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000011U };
} // namespace m

#endif // M_IDERIVEDGEN_H_
