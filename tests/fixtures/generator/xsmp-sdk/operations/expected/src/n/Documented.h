// -----------------------------------------------------------------------------
// File Name    : Documented.h
// -----------------------------------------------------------------------------
/// @file n/Documented.h

#ifndef N_DOCUMENTED_H_
#define N_DOCUMENTED_H_

// Include the generated header file
#include <n/DocumentedGen.h>

namespace n
{
class Documented: public DocumentedGen {
public:
    /// Re-use parent constructor
    using DocumentedGen::DocumentedGen;

    /// Virtual destructor to release memory.
    ~Documented() noexcept override = default;

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

    /// Compute.
    /// @param value The input value.
    /// @return Whether the value is accepted.
    ::Smp::Bool check(::Smp::Int32 value) override;
};
} // namespace n

#endif // N_DOCUMENTED_H_
