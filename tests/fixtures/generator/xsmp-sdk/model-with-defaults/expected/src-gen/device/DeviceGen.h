// -----------------------------------------------------------------------------
// File Name    : DeviceGen.h
// -----------------------------------------------------------------------------
/// @file device/DeviceGen.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef DEVICE_DEVICEGEN_H_
#define DEVICE_DEVICEGEN_H_

// ----------------------------------------------------------------------------
// --------------------------- Forward Declarations ---------------------------
// ----------------------------------------------------------------------------

namespace device
{
class ITarget;
} // namespace device

namespace device
{
class Device;
} // namespace device

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <device/State.h>
#include <functional>
#include <map>
#include <Smp/IComposite.h>
#include <Smp/IRequest.h>
#include <Smp/ISimulator.h>
#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <string_view>
#include <Xsmp/Field.h>
#include <Xsmp/Model.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace device
{
/// Universally unique identifier of type Device.
inline constexpr ::Smp::Uuid Uuid_Device { 0x33333333U, 0x3333U, 0x4333U, 0x8333U, 0x333333333333U };

class DeviceGen: public ::Xsmp::Model
{
    friend class ::device::Device;
    public:
    /// Constructor setting name, description, parent and simulator.
    /// @param name Name of new Model instance.
    /// @param description Description of new Model instance.
    /// @param parent Parent of new Model instance.
    /// @param simulator The simulator instance.
    DeviceGen(::Smp::String8 name,
            ::Smp::String8 description,
            ::Smp::IComposite* parent,
            ::Smp::ISimulator* simulator);
    /// deleted copy constructor
    DeviceGen(const DeviceGen&) = delete;
    /// deleted move constructor
    DeviceGen(DeviceGen&&) = delete;
    /// deleted copy assignment
    DeviceGen& operator=(const DeviceGen&) = delete;
    /// deleted move assignment
    DeviceGen& operator=(DeviceGen&&) = delete;

    /// Virtual destructor to release memory.
    ~DeviceGen() override = default;

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
    static std::map<std::string_view, std::function<void(DeviceGen*, ::Smp::IRequest*)>> _requestHandlers;

    public:
    /// Dynamically invoke an operation using a request object that has
    /// been created and filled with parameter values by the caller.
    /// @param   request Request object to invoke.
    /// @throws  Smp::InvalidOperationName
    /// @throws  Smp::InvalidParameterCount
    /// @throws  Smp::InvalidParameterType
    void Invoke(::Smp::IRequest* request) override;

    private:
    /// Get enabled.
    /// @return Current value of property enabled.
    virtual ::Smp::Bool get_enabled()=0;
    /// Set enabled.
    /// @param value New value of property enabled to set.
    virtual void set_enabled(::Smp::Bool value)=0;
    /// Get immutable.
    /// @return Current value of property immutable.
    virtual ::Smp::Bool get_immutable() const=0;
    virtual ::Smp::Bool ready()=0;
    virtual void setLabel(::Smp::String8 value)=0;
    virtual ::device::ITarget* target()=0;
    ::Xsmp::Field<::device::State>::output state;
};
} // namespace device

#endif // DEVICE_DEVICEGEN_H_
