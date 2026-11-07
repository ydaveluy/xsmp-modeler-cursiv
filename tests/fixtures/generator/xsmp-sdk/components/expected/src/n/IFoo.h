// -----------------------------------------------------------------------------
// File Name    : IFoo.h
// -----------------------------------------------------------------------------
/// @file n/IFoo.h

#ifndef N_IFOO_H_
#define N_IFOO_H_

// Include the generated header file
#include <n/IFooGen.h>

namespace n
{
class IFoo: public IFooGen {
public:
    ~IFoo() override = default;
};
} // namespace n

#endif // N_IFOO_H_
