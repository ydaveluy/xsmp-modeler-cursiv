// -----------------------------------------------------------------------------
// File Name    : C.h
// -----------------------------------------------------------------------------
/// @file n/C.h

#ifndef N_C_H_
#define N_C_H_

// Include the generated header file
#include <n/CGen.h>

namespace n
{
class C: public CGen
{
    friend class ::n::CGen;
    public:
    C() = default;
    C(const C&) = default;
    ::Smp::Int32 compute() override;
};
} // namespace n

#endif // N_C_H_
