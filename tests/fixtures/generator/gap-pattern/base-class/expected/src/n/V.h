// The attribute is only metadata: it changes nothing in the generated code.
// -----------------------------------------------------------------------------
// File Name    : V.h
// -----------------------------------------------------------------------------
/// @file n/V.h

#ifndef N_V_H_
#define N_V_H_

// Include the generated header file
#include <n/VGen.h>

namespace n
{
class V: public VGen {
public:
    /// Re-use parent constructor
    using VGen::VGen;

    /// Virtual destructor to release memory.
    ~V() noexcept override = default;

private:
    // visibility to call DoPublish / DoConfigure / DoConnect / DoDisconnect
    friend class ::Xsmp::Component::Helper;

    /// Publish fields, operations and properties of the Service.
    /// @param receiver Publication receiver.
    void DoPublish(::Smp::IPublication* receiver);

    /// Perform any custom configuration of the Service.
    /// @param logger Logger to use for log messages during Configure().
    /// @param linkRegistry Link Registry to use for registration of
    ///         links created during Configure() or later.
    void DoConfigure(::Smp::Services::ILogger* logger, ::Smp::Services::ILinkRegistry* linkRegistry);

    /// Connect the Service to the simulator and its simulation
    /// services.
    /// @param simulator Simulation Environment that hosts the Service.
    void DoConnect(::Smp::ISimulator* simulator);

    /// Disconnect the Service from the simulator and all its
    /// simulation services.
    void DoDisconnect();
};
} // namespace n

#endif // N_V_H_
