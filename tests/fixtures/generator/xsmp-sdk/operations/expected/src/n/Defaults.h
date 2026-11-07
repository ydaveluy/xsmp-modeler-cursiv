// -----------------------------------------------------------------------------
// File Name    : Defaults.h
// -----------------------------------------------------------------------------
/// @file n/Defaults.h

#ifndef N_DEFAULTS_H_
#define N_DEFAULTS_H_

// Include the generated header file
#include <n/DefaultsGen.h>

namespace n
{
class Defaults: public DefaultsGen {
public:
    /// Re-use parent constructor
    using DefaultsGen::DefaultsGen;

    /// Virtual destructor to release memory.
    ~Defaults() noexcept override = default;

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

    void lost(::Smp::Int32 a = 1, ::Smp::Int32* b = nullptr) override;
    void kept(::Smp::Int32* a, ::Smp::Int32 b = 3) override;
    void pointer(::Smp::Int32* a = nullptr) override;
};
} // namespace n

#endif // N_DEFAULTS_H_
