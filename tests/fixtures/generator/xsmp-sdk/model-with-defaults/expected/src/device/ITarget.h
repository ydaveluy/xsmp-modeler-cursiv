// -----------------------------------------------------------------------------
// File Name    : ITarget.h
// -----------------------------------------------------------------------------
/// @file device/ITarget.h

#ifndef DEVICE_ITARGET_H_
#define DEVICE_ITARGET_H_

// Include the generated header file
#include <device/ITargetGen.h>

namespace device
{
class ITarget: public ITargetGen {
public:
    ~ITarget() override = default;
};
} // namespace device

#endif // DEVICE_ITARGET_H_
