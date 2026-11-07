// -----------------------------------------------------------------------------
// File Name    : NGen.h
// -----------------------------------------------------------------------------
/// @file n/NGen.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef N_NGEN_H_
#define N_NGEN_H_

// ----------------------------------------------------------------------------
// --------------------------- Forward Declarations ---------------------------
// ----------------------------------------------------------------------------

namespace n
{
class I;
} // namespace n

namespace n
{
class N;
} // namespace n

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <n/S.h>
#include <Smp/IComposite.h>
#include <Smp/ISimulator.h>
#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <Xsmp/Model.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace n
{
/// Universally unique identifier of type N.
inline constexpr ::Smp::Uuid Uuid_N { 0x11111111U, 0x0000U, 0x4000U, 0x8000U, 0x0000000000e3U };

class NGen: public ::Xsmp::Model
{
    friend class ::n::N;
    public:
    /// Constructor setting name, description, parent and simulator.
    /// @param name Name of new Model instance.
    /// @param description Description of new Model instance.
    /// @param parent Parent of new Model instance.
    /// @param simulator The simulator instance.
    NGen(::Smp::String8 name,
            ::Smp::String8 description,
            ::Smp::IComposite* parent,
            ::Smp::ISimulator* simulator);
    /// deleted copy constructor
    NGen(const NGen&) = delete;
    /// deleted move constructor
    NGen(NGen&&) = delete;
    /// deleted copy assignment
    NGen& operator=(const NGen&) = delete;
    /// deleted move assignment
    NGen& operator=(NGen&&) = delete;

    /// Virtual destructor to release memory.
    ~NGen() override = default;

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
    /// Get value.
    /// @return Current value of property value.
    virtual ::n::S& get_value()=0;
    /// Set value.
    /// @param value New value of property value to set.
    virtual void set_value(::n::S& value)=0;
    virtual void by(const ::n::I& i)=0;
};
} // namespace n

#endif // N_NGEN_H_
