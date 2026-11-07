// -----------------------------------------------------------------------------
// File Name    : S.h
// -----------------------------------------------------------------------------
/// @file n/S.h

#ifndef N_S_H_
#define N_S_H_

// Include the generated header file
#include <n/SGen.h>

namespace n
{
class S: public SGen {
public:
    /// Re-use parent constructor
    using SGen::SGen;

    /// Virtual destructor to release memory.
    ~S() noexcept override = default;

private:
    // SGen call DoPublish/DoConfigure/DoConnect/DoDisconnect
    friend class ::n::SGen;

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

    void h() override;
};
} // namespace n

#endif // N_S_H_
