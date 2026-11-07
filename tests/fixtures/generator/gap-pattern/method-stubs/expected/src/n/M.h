// -----------------------------------------------------------------------------
// File Name    : M.h
// -----------------------------------------------------------------------------
/// @file n/M.h

#ifndef N_M_H_
#define N_M_H_

// Include the generated header file
#include <n/MGen.h>

namespace n
{
class M: public MGen {
public:
    /// Re-use parent constructor
    using MGen::MGen;

    /// Virtual destructor to release memory.
    ~M() noexcept override = default;

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

    ::Smp::Int32& get_value() override;
    void set_value(::Smp::Int32& value) override;
    ::n::Huge huge() override;
    ::n::S structure() override;
    ::n::E literal() override;
    ::Smp::Int32 number() override;
    ::Smp::Int32& reference() override;
};
} // namespace n

#endif // N_M_H_
