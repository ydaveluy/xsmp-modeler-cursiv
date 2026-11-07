// -----------------------------------------------------------------------------
// File Name    : M1.h
// -----------------------------------------------------------------------------
/// @file c/M1.h

#ifndef C_M1_H_
#define C_M1_H_

// Include the generated header file
#include <c/M1Gen.h>

namespace c
{
/// See {@link T1} inline and {@link T2 two}.
class M1: public M1Gen {
public:
    /// Re-use parent constructor
    using M1Gen::M1Gen;

    /// Virtual destructor to release memory.
    ~M1() noexcept override = default;

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

    /// Does things.
    /// @param a The first, see {@link T1}.
    /// @param b The second
    ///          on two lines.
    /// @param c   Spaced.
    /// @return The result, it's fine.
    /// @throws c.Oops never
    ::Smp::Int32 f(::Smp::Int32 a, ::Smp::Int32 b, ::Smp::Int32 c) override;
    /// @param x only a tag
    void noText(::Smp::Int32 x) override;
};
} // namespace c

#endif // C_M1_H_
