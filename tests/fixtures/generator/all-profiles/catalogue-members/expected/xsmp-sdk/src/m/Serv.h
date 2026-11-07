// Block notice
// line two
// -----------------------------------------------------------------------------
// File Name    : Serv.h
// -----------------------------------------------------------------------------
/// @file m/Serv.h

#ifndef M_SERV_H_
#define M_SERV_H_

// Include the generated header file
#include <m/ServGen.h>

namespace m
{
/// A service.
class Serv: public ServGen {
public:
    /// Re-use parent constructor
    using ServGen::ServGen;

    /// Virtual destructor to release memory.
    ~Serv() noexcept override = default;

private:
    // visibility to call DoPublish / DoConfigure / DoConnect / DoDisconnect
    friend class ::Xsmp::Component::Helper;

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

    ::Smp::Int32 get_ro() override;
    void set_wo(::Smp::Int32 value) override;
    ::Smp::Bool ping(::Smp::Int32 value) override;
    public:
    void _go() override;
    void _bang(::Smp::IObject* sender) override;
};
} // namespace m

#endif // M_SERV_H_
