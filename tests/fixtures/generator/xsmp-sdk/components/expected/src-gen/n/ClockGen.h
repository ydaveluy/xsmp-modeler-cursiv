// -----------------------------------------------------------------------------
// File Name    : ClockGen.h
// -----------------------------------------------------------------------------
/// @file n/ClockGen.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef N_CLOCKGEN_H_
#define N_CLOCKGEN_H_

// ----------------------------------------------------------------------------
// --------------------------- Forward Declarations ---------------------------
// ----------------------------------------------------------------------------

namespace n
{
class Clock;
} // namespace n

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <Smp/IComposite.h>
#include <Smp/ISimulator.h>
#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <Xsmp/Service.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace n
{
/// Universally unique identifier of type Clock.
inline constexpr ::Smp::Uuid Uuid_Clock { 0x11111111U, 0x0000U, 0x4000U, 0x8000U, 0x000000000065U };

class ClockGen: public ::Xsmp::Service
{
    friend class ::n::Clock;
    public:
    /// Constructor setting name, description, parent and simulator.
    /// @param name Name of new Service instance.
    /// @param description Description of new Service instance.
    /// @param parent Parent of new Service instance.
    /// @param simulator The simulator instance.
    ClockGen(::Smp::String8 name,
            ::Smp::String8 description,
            ::Smp::IComposite* parent,
            ::Smp::ISimulator* simulator);
    /// deleted copy constructor
    ClockGen(const ClockGen&) = delete;
    /// deleted move constructor
    ClockGen(ClockGen&&) = delete;
    /// deleted copy assignment
    ClockGen& operator=(const ClockGen&) = delete;
    /// deleted move assignment
    ClockGen& operator=(ClockGen&&) = delete;

    /// Virtual destructor to release memory.
    ~ClockGen() override = default;

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
} // namespace n

#endif // N_CLOCKGEN_H_
