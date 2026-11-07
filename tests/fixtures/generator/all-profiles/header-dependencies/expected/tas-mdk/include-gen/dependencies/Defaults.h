// -----------------------------------------------------------------------------
// File Name    : Defaults.h
// -----------------------------------------------------------------------------
/// @file dependencies/Defaults.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef DEPENDENCIES_DEFAULTS_H_
#define DEPENDENCIES_DEFAULTS_H_

// ----------------------------------------------------------------------------
// --------------------------- Forward Declarations ---------------------------
// ----------------------------------------------------------------------------

namespace dependencies
{
struct Value;
} // namespace dependencies

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <dependencies/Value.h>
#include <Smp/Publication/ITypeRegistry.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace dependencies
{
class Defaults
{
    public:
    static void _Register(::Smp::Publication::ITypeRegistry* registry);

    Defaults() = default;
    ~Defaults() noexcept = default;
    Defaults(const Defaults&) = default;

    private:
    void consume(::dependencies::Value item = { 1});
};

/// Universally unique identifier of type Defaults.
constexpr ::Smp::Uuid Uuid_Defaults { 0x82000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000006U };
} // namespace dependencies

#endif // DEPENDENCIES_DEFAULTS_H_
