// -----------------------------------------------------------------------------
// File Name    : SensorGen.h
// -----------------------------------------------------------------------------
/// @file n/SensorGen.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef N_SENSORGEN_H_
#define N_SENSORGEN_H_

// ----------------------------------------------------------------------------
// --------------------------- Forward Declarations ---------------------------
// ----------------------------------------------------------------------------

namespace n
{
class Sensor;
} // namespace n

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <functional>
#include <map>
#include <n/Child.h>
#include <n/IFoo.h>
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
#include <Xsmp/Model.h>
#include <Xsmp/Reference.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace n
{
/// Universally unique identifier of type Sensor.
inline constexpr ::Smp::Uuid Uuid_Sensor { 0x11111111U, 0x0000U, 0x4000U, 0x8000U, 0x000000000064U };

class SensorGen: public ::Xsmp::Model, public virtual ::n::IFoo, public virtual ::Xsmp::Composite, public virtual ::Xsmp::Aggregate, public virtual ::Xsmp::EventProvider, public virtual ::Xsmp::EventConsumer, public virtual ::Xsmp::EntryPointPublisher
{
    friend class ::n::Sensor;
    public:
    /// Constructor setting name, description, parent and simulator.
    /// @param name Name of new Model instance.
    /// @param description Description of new Model instance.
    /// @param parent Parent of new Model instance.
    /// @param simulator The simulator instance.
    SensorGen(::Smp::String8 name,
            ::Smp::String8 description,
            ::Smp::IComposite* parent,
            ::Smp::ISimulator* simulator);
    /// deleted copy constructor
    SensorGen(const SensorGen&) = delete;
    /// deleted move constructor
    SensorGen(SensorGen&&) = delete;
    /// deleted copy assignment
    SensorGen& operator=(const SensorGen&) = delete;
    /// deleted move assignment
    SensorGen& operator=(SensorGen&&) = delete;

    /// Virtual destructor to release memory.
    ~SensorGen() override = default;

    /// Request the Model to publish its fields, properties and
    /// operations against the provided publication receiver.
    /// @param   receiver Publication receiver.
    /// @throws  Smp::InvalidComponentState
    void Publish(::Smp::IPublication* receiver) override;

    /// Request the Model to perform any custom configuration. The
    /// component can create and configure other components using the field
    /// values of its published fields.
    /// @param   logger Logger service for logging of error messages during
    ///          configuration.
    /// @param   linkRegistry Reference to the link registry services, so
    ///          that the Model can register links that it creates
    ///          during configuration.
    /// @throws  Smp::InvalidComponentState
    void Configure(::Smp::Services::ILogger* logger, ::Smp::Services::ILinkRegistry* linkRegistry) override;

    /// Allow the Model to connect to the simulator and its simulation
    /// services.
    /// @param   simulator Simulation Environment that hosts the Model.
    /// @throws  Smp::InvalidComponentState
    void Connect(::Smp::ISimulator* simulator) override;

    /// Ask the Model to disconnect from the simulator and all its
    /// simulation services.
    /// @throws  Smp::InvalidComponentState
    void Disconnect() override;

    /// Get Universally Unique Identifier of the Model.
    /// @return  Universally Unique Identifier of the Model.
    const ::Smp::Uuid& GetUuid() const override;
    private:
    static std::map<std::string_view, std::function<void(SensorGen*, ::Smp::IRequest*)>> _requestHandlers;

    public:
    /// Dynamically invoke an operation using a request object that has
    /// been created and filled with parameter values by the caller.
    /// @param   request Request object to invoke.
    /// @throws  Smp::InvalidOperationName
    /// @throws  Smp::InvalidParameterCount
    /// @throws  Smp::InvalidParameterType
    void Invoke(::Smp::IRequest* request) override;

    private:
    /// Get Value.
    /// @return Current value of property Value.
    virtual ::Smp::Float64 get_Value();
    /// Set Value.
    /// @param value New value of property Value to set.
    virtual void set_Value(::Smp::Float64 value);
    virtual void doIt()=0;
    public:
    ::Xsmp::EntryPoint step;
    virtual void _step() = 0;
    ::Xsmp::EventSink<> sink;
    virtual void _sink(::Smp::IObject* sender) = 0;
    ::Xsmp::EventSource<> source;
    private:
    ::Smp::Float64 raw;
    public:
    ::Xsmp::Container<::n::Child> slot;
    ::Xsmp::Reference<::n::IFoo> ref;
};
} // namespace n

#endif // N_SENSORGEN_H_
