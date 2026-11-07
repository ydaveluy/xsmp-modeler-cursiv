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

#include <esa/ecss/smp/cdk/Aggregate.h>
#include <esa/ecss/smp/cdk/Composite.h>
#include <esa/ecss/smp/cdk/Container.h>
#include <esa/ecss/smp/cdk/EntryPointPublisher.h>
#include <esa/ecss/smp/cdk/EventConsumer.h>
#include <esa/ecss/smp/cdk/EventProvider.h>
#include <esa/ecss/smp/cdk/EventSourceVoid.h>
#include <esa/ecss/smp/cdk/Reference.h>
#include <esa/ecss/smp/cdk/Service.h>
#include <m/IBase.h>
#include <Smp/IComposite.h>
#include <Smp/IEntryPoint.h>
#include <Smp/ISimulator.h>
#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/ITypeRegistry.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace m
{
/// Universally unique identifier of type Serv.
constexpr ::Smp::Uuid Uuid_Serv { 0x20000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000022U };

/// A service.
class ServGen: public ::esa::ecss::smp::cdk::Service, public virtual ::m::IBase, public virtual ::esa::ecss::smp::cdk::Composite, public virtual ::esa::ecss::smp::cdk::Aggregate, public virtual ::esa::ecss::smp::cdk::EventProvider, public virtual ::esa::ecss::smp::cdk::EventConsumer, public virtual ::esa::ecss::smp::cdk::EntryPointPublisher
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
    ~ServGen() override;

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
    ::Smp::IEntryPoint* go;
    virtual void _go() = 0;
    ::Smp::IEventSink* bang;
    virtual void _bang(::Smp::IObject* sender) = 0;
    ::esa::ecss::smp::cdk::EventSourceVoid *boom;
    private:
    ::Smp::Bool connectedState;
    public:
    ::esa::ecss::smp::cdk::Container<::Smp::IComposite>* kids;
    ::esa::ecss::smp::cdk::Reference<::m::IBase>* peer;
};
} // namespace m

#endif // M_SERVGEN_H_
