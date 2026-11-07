// -----------------------------------------------------------------------------
// File Name    : ConstantsGen.h
// -----------------------------------------------------------------------------
/// @file order/ConstantsGen.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef ORDER_CONSTANTSGEN_H_
#define ORDER_CONSTANTSGEN_H_

// ----------------------------------------------------------------------------
// --------------------------- Forward Declarations ---------------------------
// ----------------------------------------------------------------------------

namespace order
{
class Constants;
} // namespace order

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <order/External.h>
#include <Smp/PrimitiveTypes.h>
#include <Smp/Uuid.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace order
{
class ConstantsGen {
public:
    virtual ~ConstantsGen() = default;
    static constexpr ::Smp::Int32 Independent{::order::External::Value};
    static constexpr ::Smp::Int32 First{1};
    static constexpr ::Smp::Int32 Middle{First + 1};
    static constexpr ::Smp::Int32 Last{Middle + First + Middle};
};

/// Universally unique identifier of type Constants.
constexpr ::Smp::Uuid Uuid_Constants { 0x83000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000009U };
} // namespace order

#endif // ORDER_CONSTANTSGEN_H_
