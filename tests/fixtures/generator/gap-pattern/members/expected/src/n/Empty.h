// -----------------------------------------------------------------------------
// File Name    : Empty.h
// -----------------------------------------------------------------------------
/// @file n/Empty.h

#ifndef N_EMPTY_H_
#define N_EMPTY_H_

// Include the generated header file
#include <n/EmptyGen.h>

namespace n
{
class Empty: public EmptyGen {
public:
    ~Empty() override = default;
};
} // namespace n

#endif // N_EMPTY_H_
