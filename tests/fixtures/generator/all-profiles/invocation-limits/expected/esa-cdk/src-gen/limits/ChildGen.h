// -----------------------------------------------------------------------------
// File Name    : ChildGen.h
// -----------------------------------------------------------------------------
/// @file limits/ChildGen.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef LIMITS_CHILDGEN_H_
#define LIMITS_CHILDGEN_H_

// ----------------------------------------------------------------------------
// --------------------------- Forward Declarations ---------------------------
// ----------------------------------------------------------------------------

namespace limits
{
class Child;
} // namespace limits

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <limits/Probe.h>
#include <Smp/IComposite.h>
#include <Smp/ISimulator.h>
#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/ITypeRegistry.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace limits
{
/// Universally unique identifier of type Child.
constexpr ::Smp::Uuid Uuid_Child { 0x87000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000002U };

class ChildGen: public ::limits::Probe
{
    friend class ::limits::Child;
    public:
    /// Constructor setting name, description, parent and simulator.
    /// @param name Name of new Model instance.
    /// @param description Description of new Model instance.
    /// @param parent Parent of new Model instance.
    /// @param simulator The simulator instance.
    ChildGen(::Smp::String8 name,
            ::Smp::String8 description,
            ::Smp::IComposite* parent,
            ::Smp::ISimulator* simulator);
    /// deleted copy constructor
    ChildGen(const ChildGen&) = delete;
    /// deleted move constructor
    ChildGen(ChildGen&&) = delete;
    /// deleted copy assignment
    ChildGen& operator=(const ChildGen&) = delete;
    /// deleted move assignment
    ChildGen& operator=(ChildGen&&) = delete;

    /// Virtual destructor to release memory.
    ~ChildGen() override;

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
    virtual ::Smp::Int32 normal(::Smp::Float64 value)=0;
    virtual ::Smp::Int32 fresh()=0;
};
} // namespace limits

#endif // LIMITS_CHILDGEN_H_
