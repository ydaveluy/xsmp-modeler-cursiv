// -----------------------------------------------------------------------------
// File Name    : Constructed.h
// -----------------------------------------------------------------------------
/// @file n/Constructed.h

#ifndef N_CONSTRUCTED_H_
#define N_CONSTRUCTED_H_

// Include the generated header file
#include <n/ConstructedGen.h>

namespace n
{
class Constructed: public ConstructedGen
{
    friend class ::n::ConstructedGen;
    public:
    Constructed() = default;
    Constructed(const Constructed&) = default;
    using ConstructedGen::ConstructedGen;
};
} // namespace n

#endif // N_CONSTRUCTED_H_
