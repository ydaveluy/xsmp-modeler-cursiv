// -----------------------------------------------------------------------------
// File Name    : Probe.h
// -----------------------------------------------------------------------------
/// @file optional/Probe.h

#ifndef OPTIONAL_PROBE_H_
#define OPTIONAL_PROBE_H_

// Include the generated header file
#include <optional/ProbeGen.h>

namespace optional
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

    /// Optional input.
    void ope_input(::Smp::Int32* value = nullptr) override;
    /// The request owns the String8 until the handler returns.
    void ope_string(::Smp::String8* value = nullptr) override;
    /// Optional output.
    void ope_output(::Smp::Int32* value = nullptr) override;
    /// Optional input/output.
    void ope_inout(::Smp::Int32* value = nullptr) override;
    /// The parameter names must not collide with generated presence flags.
    void ope_names(::Smp::Int32* value = nullptr, ::Smp::Int32* value_present = nullptr) override;
};
} // namespace optional

#endif // OPTIONAL_PROBE_H_
