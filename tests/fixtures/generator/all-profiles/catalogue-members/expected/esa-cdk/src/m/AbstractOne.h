// Block notice
// line two
// -----------------------------------------------------------------------------
// File Name    : AbstractOne.h
// -----------------------------------------------------------------------------
/// @file m/AbstractOne.h

#ifndef M_ABSTRACTONE_H_
#define M_ABSTRACTONE_H_

// Include the generated header file
#include <m/AbstractOneGen.h>

namespace m
{
/// A model.
class AbstractOne: public AbstractOneGen {
public:
    /// Re-use parent constructor
    using AbstractOneGen::AbstractOneGen;

    /// Virtual destructor to release memory.
    ~AbstractOne() noexcept override = default;

private:
    // AbstractOneGen call DoPublish/DoConfigure/DoConnect/DoDisconnect
    friend class ::m::AbstractOneGen;

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

    ::Smp::Bool ping(::Smp::Int32 value) override;
};
} // namespace m

#endif // M_ABSTRACTONE_H_
