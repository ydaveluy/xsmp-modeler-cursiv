// -----------------------------------------------------------------------------
// File Name    : External.h
// -----------------------------------------------------------------------------
/// @file order/External.h

#ifndef ORDER_EXTERNAL_H_
#define ORDER_EXTERNAL_H_

// Include the generated header file
#include <order/ExternalGen.h>

namespace order
{
class External: public ExternalGen {
public:
    ~External() override = default;
};
} // namespace order

#endif // ORDER_EXTERNAL_H_
