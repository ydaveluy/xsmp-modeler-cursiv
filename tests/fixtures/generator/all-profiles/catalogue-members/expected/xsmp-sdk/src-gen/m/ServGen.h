// Block notice
// line two
// -----------------------------------------------------------------------------
// File Name    : ServGen.h
// -----------------------------------------------------------------------------
/// @file m/ServGen.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef M_SERVGEN_H_
#define M_SERVGEN_H_

// ----------------------------------------------------------------------------
// --------------------------- Forward Declarations ---------------------------
// ----------------------------------------------------------------------------

namespace m
{
class Serv;
} // namespace m

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <functional>
#include <m/IBase.h>
#include <map>
#include <Smp/IComposite.h>
#include <Smp/IRequest.h>
#include <Smp/ISimulator.h>
#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <string_view>
#include <Xsmp/Aggregate.h>
#include <Xsmp/Composite.h>
#include <Xsmp/Container.h>
#include <Xsmp/EntryPoint.h>
#include <Xsmp/EntryPointPublisher.h>
#include <Xsmp/EventConsumer.h>
#include <Xsmp/EventProvider.h>
#include <Xsmp/EventSink.h>
#include <Xsmp/EventSource.h>
#include <Xsmp/Reference.h>
#include <Xsmp/Service.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace m
{
/// Universally unique identifier of type Serv.
inline constexpr ::Smp::Uuid Uuid_Serv { 0x20000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000022U };

/// A service.
class ServGen: public ::Xsmp::Service, public virtual ::m::IBase, public virtual ::Xsmp::Composite, public virtual ::Xsmp::Aggregate, public virtual ::Xsmp::EventProvider, public virtual ::Xsmp::EventConsumer, public virtual ::Xsmp::EntryPointPublisher
{
    friend class ::m::Serv;
    public:
    /// Constructor setting name, description, parent and simulator.
    /// @param name Name of new Service instance.
    /// @param description Description of new Service instance.
    /// @param parent Parent of new Service instance.
    /// @param simulator The simulator instance.
    ServGen(::Smp::String8 name,
            ::Smp::String8 description,
            ::Smp::IComposite* parent,
            ::Smp::ISimulator* simulator);
    /// deleted copy constructor
    ServGen(const ServGen&) = delete;
    /// deleted move constructor
    ServGen(ServGen&&) = delete;
    /// deleted copy assignment
    ServGen& operator=(const ServGen&) = delete;
    /// deleted move assignment
    ServGen& operator=(ServGen&&) = delete;

    /// Virtual destructor to release memory.
    ~ServGen() override = default;

    /// Request the Service to publish its fields, properties and
    /// operations against the provided publication receiver.
    /// @param   receiver Publication receiver.
    /// @throws  Smp::InvalidComponentState
    void Publish(::Smp::IPublication* receiver) override;

    /// Request the Service to perform any custom configuration. The
    /// component can create and configure other components using the field
    /// values of its published fields.
    /// @param   logger Logger service for logging of error messages during
    ///          configuration.
    /// @param   linkRegistry Reference to the link registry services, so
    ///          that the Service can register links that it creates
    ///          during configuration.
    /// @throws  Smp::InvalidComponentState
    void Configure(::Smp::Services::ILogger* logger, ::Smp::Services::ILinkRegistry* linkRegistry) override;

    /// Allow the Service to connect to the simulator and its simulation
    /// services.
    /// @param   simulator Simulation Environment that hosts the Service.
    /// @throws  Smp::InvalidComponentState
    void Connect(::Smp::ISimulator* simulator) override;

    /// Ask the Service to disconnect from the simulator and all its
    /// simulation services.
    /// @throws  Smp::InvalidComponentState
    void Disconnect() override;

    /// Get Universally Unique Identifier of the Service.
    /// @return  Universally Unique Identifier of the Service.
    const ::Smp::Uuid& GetUuid() const override;
    private:
    static std::map<std::string_view, std::function<void(ServGen*, ::Smp::IRequest*)>> _requestHandlers;

    public:
    /// Dynamically invoke an operation using a request object that has
    /// been created and filled with parameter values by the caller.
    /// @param   request Request object to invoke.
    /// @throws  Smp::InvalidOperationName
    /// @throws  Smp::InvalidParameterCount
    /// @throws  Smp::InvalidParameterType
    void Invoke(::Smp::IRequest* request) override;

    private:
    /// Get connected.
    /// @return Current value of property connected.
    virtual ::Smp::Bool get_connected();
    /// Set connected.
    /// @param value New value of property connected to set.
    virtual void set_connected(::Smp::Bool value);
    /// Get ro.
    /// @return Current value of property ro.
    virtual ::Smp::Int32 get_ro()=0;
    /// Set wo.
    /// @param value New value of property wo to set.
    virtual void set_wo(::Smp::Int32 value)=0;
    virtual ::Smp::Bool ping(::Smp::Int32 value)=0;
    public:
    ::Xsmp::EntryPoint go;
    virtual void _go() = 0;
    ::Xsmp::EventSink<> bang;
    virtual void _bang(::Smp::IObject* sender) = 0;
    ::Xsmp::EventSource<> boom;
    private:
    ::Smp::Bool connectedState;
    public:
    ::Xsmp::Container<::Smp::IComposite> kids;
    ::Xsmp::Reference<::m::IBase> peer;
};
} // namespace m

#endif // M_SERVGEN_H_
