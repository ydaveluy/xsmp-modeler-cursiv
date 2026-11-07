// -----------------------------------------------------------------------------
// File Name    : NullDefaults.h
// -----------------------------------------------------------------------------
/// @file n/NullDefaults.h

#ifndef N_NULLDEFAULTS_H_
#define N_NULLDEFAULTS_H_

// Include the generated header file
#include <n/NullDefaultsGen.h>

namespace n
{
class NullDefaults: public NullDefaultsGen {
public:
    /// Re-use parent constructor
    using NullDefaultsGen::NullDefaultsGen;

    /// Virtual destructor to release memory.
    ~NullDefaults() noexcept override = default;

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

    void f(::Smp::Duration* a = nullptr, ::Smp::Int32* b = nullptr) override;
    void g(::Smp::Int32* c = nullptr) override;
    void h(::Smp::Int32* d = nullptr, ::Smp::Int32* e = nullptr) override;
};
} // namespace n

#endif // N_NULLDEFAULTS_H_
