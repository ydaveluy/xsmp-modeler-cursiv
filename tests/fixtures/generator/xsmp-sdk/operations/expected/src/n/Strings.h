// -----------------------------------------------------------------------------
// File Name    : Strings.h
// -----------------------------------------------------------------------------
/// @file n/Strings.h

#ifndef N_STRINGS_H_
#define N_STRINGS_H_

// Include the generated header file
#include <n/StringsGen.h>

namespace n
{
class Strings: public StringsGen {
public:
    /// Re-use parent constructor
    using StringsGen::StringsGen;

    /// Virtual destructor to release memory.
    ~Strings() noexcept override = default;

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

    ::Smp::Bool get_enabled() override;
    void set_enabled(::Smp::Bool value) override;
    void inputString(::Smp::String8 value) override;
    void outputString(::Smp::String8* value) override;
    void inoutString(::Smp::String8* value) override;
    ::Smp::String8 returnString() override;
    ::Smp::Bool returnBool() override;
};
} // namespace n

#endif // N_STRINGS_H_
