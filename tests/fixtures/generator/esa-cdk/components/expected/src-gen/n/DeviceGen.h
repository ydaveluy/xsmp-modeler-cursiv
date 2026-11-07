// -----------------------------------------------------------------------------
// File Name    : DeviceGen.h
// -----------------------------------------------------------------------------
/// @file n/DeviceGen.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef N_DEVICEGEN_H_
#define N_DEVICEGEN_H_

// ----------------------------------------------------------------------------
// --------------------------- Forward Declarations ---------------------------
// ----------------------------------------------------------------------------

namespace n
{
class Device;
} // namespace n

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <esa/ecss/smp/cdk/Model.h>
#include <Smp/IComposite.h>
#include <Smp/ISimulator.h>
#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/ITypeRegistry.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace n
{
/// Universally unique identifier of type Device.
constexpr ::Smp::Uuid Uuid_Device { 0x11111111U, 0x0000U, 0x4000U, 0x8000U, 0x000000000121U };

class DeviceGen: public ::esa::ecss::smp::cdk::Model
{
    friend class ::n::Device;
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
    ~DeviceGen() override;

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
    virtual void invisible()=0;
    virtual void measure(::Smp::Float64* value)=0;
};
} // namespace n

#endif // N_DEVICEGEN_H_
