// -----------------------------------------------------------------------------
// File Name    : A.h
// -----------------------------------------------------------------------------
/// @file n/A.h

#ifndef N_A_H_
#define N_A_H_

// Include the generated header file
#include <n/AGen.h>

namespace n
{
class A: public AGen {
public:
    /// Re-use parent constructor
    using AGen::AGen;

    /// Virtual destructor to release memory.
    ~A() noexcept override = default;

private:
    // AGen call DoPublish/DoConfigure/DoConnect/DoDisconnect
    friend class ::n::AGen;

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

    void f() override;
};
} // namespace n

#endif // N_A_H_
