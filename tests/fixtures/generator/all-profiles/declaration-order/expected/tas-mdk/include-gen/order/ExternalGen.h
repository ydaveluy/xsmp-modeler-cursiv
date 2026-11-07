// -----------------------------------------------------------------------------
// File Name    : ExternalGen.h
// -----------------------------------------------------------------------------
/// @file order/ExternalGen.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef ORDER_EXTERNALGEN_H_
#define ORDER_EXTERNALGEN_H_

// ----------------------------------------------------------------------------
// --------------------------- Forward Declarations ---------------------------
// ----------------------------------------------------------------------------

namespace order
{
class External;
} // namespace order

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <Smp/PrimitiveTypes.h>
#include <Smp/Uuid.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace order
{
class ExternalGen {
public:
    virtual ~ExternalGen() = default;
    static constexpr ::Smp::Int32 Value{4};
};

/// Universally unique identifier of type External.
constexpr ::Smp::Uuid Uuid_External { 0x83000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000008U };
} // namespace order

#endif // ORDER_EXTERNALGEN_H_
