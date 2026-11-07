// -----------------------------------------------------------------------------
// File Name    : Pointers.h
// -----------------------------------------------------------------------------
/// @file n/Pointers.h

#ifndef N_POINTERS_H_
#define N_POINTERS_H_

// Include the generated header file
#include <n/PointersGen.h>

namespace n
{
class Pointers: public PointersGen {
public:
    /// Re-use parent constructor
    using PointersGen::PointersGen;

    /// Virtual destructor to release memory.
    ~Pointers() noexcept override = default;

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

    ::Smp::Int32* get_pointer() override;
    void set_pointer(::Smp::Int32* value) override;
    ::Smp::Int32& get_reference() override;
    void set_reference(::Smp::Int32& value) override;
    ::Smp::Int32 get_plain() override;
    void set_plain(::Smp::Int32 value) override;
    ::Smp::Int32* address() override;
    ::Smp::Int32& alias() override;
};
} // namespace n

#endif // N_POINTERS_H_
