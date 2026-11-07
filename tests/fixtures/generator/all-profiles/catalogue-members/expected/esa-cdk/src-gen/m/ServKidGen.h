// Block notice
// line two
// -----------------------------------------------------------------------------
// File Name    : ServKidGen.h
// -----------------------------------------------------------------------------
/// @file m/ServKidGen.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef M_SERVKIDGEN_H_
#define M_SERVKIDGEN_H_

// ----------------------------------------------------------------------------
// --------------------------- Forward Declarations ---------------------------
// ----------------------------------------------------------------------------

namespace m
{
class ServKid;
} // namespace m

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <m/Serv.h>
#include <Smp/IComposite.h>
#include <Smp/ISimulator.h>
#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/ITypeRegistry.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace m
{
/// Universally unique identifier of type ServKid.
constexpr ::Smp::Uuid Uuid_ServKid { 0x20000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000023U };

class ServKidGen: public ::m::Serv
{
    friend class ::m::ServKid;
    public:
    /// Constructor setting name, description, parent and simulator.
    /// @param name Name of new Service instance.
    /// @param description Description of new Service instance.
    /// @param parent Parent of new Service instance.
    /// @param simulator The simulator instance.
    ServKidGen(::Smp::String8 name,
            ::Smp::String8 description,
            ::Smp::IComposite* parent,
            ::Smp::ISimulator* simulator);
    /// deleted copy constructor
    ServKidGen(const ServKidGen&) = delete;
    /// deleted move constructor
    ServKidGen(ServKidGen&&) = delete;
    /// deleted copy assignment
    ServKidGen& operator=(const ServKidGen&) = delete;
    /// deleted move assignment
    ServKidGen& operator=(ServKidGen&&) = delete;

    /// Virtual destructor to release memory.
    ~ServKidGen() override;

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
};
} // namespace m

#endif // M_SERVKIDGEN_H_
