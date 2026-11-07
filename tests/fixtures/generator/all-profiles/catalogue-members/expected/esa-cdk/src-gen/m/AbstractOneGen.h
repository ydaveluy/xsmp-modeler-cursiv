// Block notice
// line two
// -----------------------------------------------------------------------------
// File Name    : AbstractOneGen.h
// -----------------------------------------------------------------------------
/// @file m/AbstractOneGen.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef M_ABSTRACTONEGEN_H_
#define M_ABSTRACTONEGEN_H_

// ----------------------------------------------------------------------------
// --------------------------- Forward Declarations ---------------------------
// ----------------------------------------------------------------------------

namespace m
{
class AbstractOne;
} // namespace m

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <esa/ecss/smp/cdk/Model.h>
#include <m/IBase.h>
#include <Smp/IComposite.h>
#include <Smp/ISimulator.h>
#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/ITypeRegistry.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace m
{
/// Universally unique identifier of type AbstractOne.
constexpr ::Smp::Uuid Uuid_AbstractOne { 0x20000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000020U };

/// A model.
class AbstractOneGen: public ::esa::ecss::smp::cdk::Model, public virtual ::m::IBase
{
    friend class ::m::AbstractOne;
    public:
    /// Constructor setting name, description, parent and simulator.
    /// @param name Name of new Model instance.
    /// @param description Description of new Model instance.
    /// @param parent Parent of new Model instance.
    /// @param simulator The simulator instance.
    AbstractOneGen(::Smp::String8 name,
            ::Smp::String8 description,
            ::Smp::IComposite* parent,
            ::Smp::ISimulator* simulator);
    /// deleted copy constructor
    AbstractOneGen(const AbstractOneGen&) = delete;
    /// deleted move constructor
    AbstractOneGen(AbstractOneGen&&) = delete;
    /// deleted copy assignment
    AbstractOneGen& operator=(const AbstractOneGen&) = delete;
    /// deleted move assignment
    AbstractOneGen& operator=(AbstractOneGen&&) = delete;

    /// Virtual destructor to release memory.
    ~AbstractOneGen() override;

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
    static constexpr ::Smp::Int32 K{2};
    /// Get connected.
    /// @return Current value of property connected.
    virtual ::Smp::Bool get_connected();
    /// Set connected.
    /// @param value New value of property connected to set.
    virtual void set_connected(::Smp::Bool value);
    /// Get ro.
    /// @return Current value of property ro.
    virtual ::Smp::Int32 get_ro();
    /// Set wo.
    /// @param value New value of property wo to set.
    virtual void set_wo(::Smp::Int32 value);
    virtual ::Smp::Int32 mustImplement()=0;
    virtual ::Smp::Bool ping(::Smp::Int32 value)=0;
    ::Smp::Bool connectedState;
    ::Smp::Int32 valueState;
};
} // namespace m

#endif // M_ABSTRACTONEGEN_H_
