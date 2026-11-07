// First license line
// Second license line
// -----------------------------------------------------------------------------
// File Name    : Device.h
// -----------------------------------------------------------------------------
/// @file demo/Device.h

#ifndef DEMO_DEVICE_H_
#define DEMO_DEVICE_H_

// Include the generated header file
#include <demo/DeviceGen.h>

namespace demo
{
class Device: public DeviceGen {
public:
    /// Re-use parent constructor
    using DeviceGen::DeviceGen;

    /// Virtual destructor to release memory.
    ~Device() noexcept override = default;

private:
    // visibility to call DoPublish / DoConfigure / DoConnect / DoDisconnect
    friend class ::Xsmp::Component::Helper;

    /// Publish fields, operations and properties of the Model.
    /// @param receiver Publication receiver.
    void DoPublish(::Smp::IPublication* receiver);

    /// Perform any custom configuration of the Model.
    /// @param logger Logger to use for log messages during Configure().
    /// @param linkRegistry Link Registry to use for registration of
    ///         links created during Configure() or later.
    void DoConfigure(::Smp::Services::ILogger* logger, ::Smp::Services::ILinkRegistry* linkRegistry);

    /// Connect the Model to the simulator and its simulation
    /// services.
    /// @param simulator Simulation Environment that hosts the Model.
    void DoConnect(::Smp::ISimulator* simulator);

    /// Disconnect the Model from the simulator and all its
    /// simulation services.
    void DoDisconnect();
};
} // namespace demo

#endif // DEMO_DEVICE_H_
