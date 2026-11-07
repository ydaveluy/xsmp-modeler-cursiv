// Block notice
// line two
// -----------------------------------------------------------------------------
// File Name    : Failed.h
// -----------------------------------------------------------------------------
/// @file m/Failed.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef M_FAILED_H_
#define M_FAILED_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <Smp/Publication/ITypeRegistry.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace m
{
class Failed: public ::Smp::Exception
{
    public:
    static void _Register(::Smp::Publication::ITypeRegistry* registry);

    Failed() = default;
    ~Failed() noexcept = default;
    Failed(const Failed&) = default;
};

/// Universally unique identifier of type Failed.
inline constexpr ::Smp::Uuid Uuid_Failed { 0x20000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000007U };
} // namespace m

#endif // M_FAILED_H_
