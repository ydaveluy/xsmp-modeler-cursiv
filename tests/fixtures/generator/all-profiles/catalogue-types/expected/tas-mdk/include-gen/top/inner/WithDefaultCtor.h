// Copyright line one
// Copyright line two
// -----------------------------------------------------------------------------
// File Name    : WithDefaultCtor.h
// -----------------------------------------------------------------------------
/// @file top/inner/WithDefaultCtor.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef TOP_INNER_WITHDEFAULTCTOR_H_
#define TOP_INNER_WITHDEFAULTCTOR_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <Smp/Publication/ITypeRegistry.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

/// Top namespace doc.
namespace top
{
/// Inner namespace doc.
///  Second line.
namespace inner
{
class WithDefaultCtor
{
    public:
    static void _Register(::Smp::Publication::ITypeRegistry* registry);

    ~WithDefaultCtor() noexcept = default;
    WithDefaultCtor(const WithDefaultCtor&) = default;

    WithDefaultCtor();
};

/// Universally unique identifier of type WithDefaultCtor.
constexpr ::Smp::Uuid Uuid_WithDefaultCtor { 0x10000000U, 0x0000U, 0x4000U, 0x8000U, 0x00000000001aU };
} // namespace inner
} // namespace top

#endif // TOP_INNER_WITHDEFAULTCTOR_H_
