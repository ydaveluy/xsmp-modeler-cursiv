// -----------------------------------------------------------------------------
// File Name    : ProbeGen.h
// -----------------------------------------------------------------------------
/// @file limits/ProbeGen.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef LIMITS_PROBEGEN_H_
#define LIMITS_PROBEGEN_H_

// ----------------------------------------------------------------------------
// --------------------------- Forward Declarations ---------------------------
// ----------------------------------------------------------------------------

namespace limits
{
class Probe;
} // namespace limits

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

namespace limits
{
/// Universally unique identifier of type Probe.
constexpr ::Smp::Uuid Uuid_Probe { 0x87000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000001U };

class ProbeGen: public ::esa::ecss::smp::cdk::Model
{
    friend class ::limits::Probe;
    public:
    /// Constructor setting name, description, parent and simulator.
    /// @param name Name of new Model instance.
    /// @param description Description of new Model instance.
    /// @param parent Parent of new Model instance.
    /// @param simulator The simulator instance.
    ProbeGen(::Smp::String8 name,
            ::Smp::String8 description,
            ::Smp::IComposite* parent,
            ::Smp::ISimulator* simulator);
    /// deleted copy constructor
    ProbeGen(const ProbeGen&) = delete;
    /// deleted move constructor
    ProbeGen(ProbeGen&&) = delete;
    /// deleted copy assignment
    ProbeGen& operator=(const ProbeGen&) = delete;
    /// deleted move assignment
    ProbeGen& operator=(ProbeGen&&) = delete;

    /// Virtual destructor to release memory.
    ~ProbeGen() override;

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
    ProbeGen(::Smp::Int32 value);
    ProbeGen(::Smp::Float64 value);
    virtual ::Smp::Int32* pointer()=0;
    virtual ::Smp::Int32 calculate(::Smp::Int32 value)=0;
    virtual ::Smp::Int32 calculate(::Smp::Float64 value)=0;
    virtual ::Smp::Int32 normal(::Smp::Int32 value)=0;
};
} // namespace limits

#endif // LIMITS_PROBEGEN_H_
