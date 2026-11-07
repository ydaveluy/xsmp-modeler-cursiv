// -----------------------------------------------------------------------------
// File Name    : Probe.h
// -----------------------------------------------------------------------------
/// @file limits/Probe.h

#ifndef LIMITS_PROBE_H_
#define LIMITS_PROBE_H_

// Include the generated header file
#include <limits/ProbeGen.h>

namespace limits
{
class Probe: public ProbeGen {
public:
    /// Re-use parent constructor
    using ProbeGen::ProbeGen;

    /// Virtual destructor to release memory.
    ~Probe() noexcept override = default;

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

    ::Smp::Int32* pointer() override;
    ::Smp::Int32 calculate(::Smp::Int32 value) override;
    ::Smp::Int32 calculate(::Smp::Float64 value) override;
    ::Smp::Int32 normal(::Smp::Int32 value) override;
};
} // namespace limits

#endif // LIMITS_PROBE_H_
