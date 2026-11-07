// -----------------------------------------------------------------------------
// File Name    : CGen.h
// -----------------------------------------------------------------------------
/// @file n/CGen.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef N_CGEN_H_
#define N_CGEN_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/ITypeRegistry.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace n
{
class C;

class CGen
{
    friend class ::n::C;
    public:
    static void _Register(::Smp::Publication::ITypeRegistry* registry);

    CGen() = default;
    ~CGen() noexcept = default;
    CGen(const CGen&) = default;

    virtual ::Smp::Int32 compute()=0;
};

/// Universally unique identifier of type C.
constexpr ::Smp::Uuid Uuid_C { 0x11111111U, 0x0000U, 0x4000U, 0x8000U, 0x000000000101U };
} // namespace n

#endif // N_CGEN_H_
