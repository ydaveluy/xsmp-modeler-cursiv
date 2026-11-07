// -----------------------------------------------------------------------------
// File Name    : Value.h
// -----------------------------------------------------------------------------
/// @file dependencies/Value.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef DEPENDENCIES_VALUE_H_
#define DEPENDENCIES_VALUE_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <Smp/ViewKind.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace dependencies
{
struct Value
{
    ::Smp::Int32 number;

    static void _Register(::Smp::Publication::ITypeRegistry* registry);
};

/// Universally unique identifier of type Value.
constexpr ::Smp::Uuid Uuid_Value { 0x82000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000005U };
} // namespace dependencies

#endif // DEPENDENCIES_VALUE_H_
