// -----------------------------------------------------------------------------
// File Name    : OutputDefaultsGen.h
// -----------------------------------------------------------------------------
/// @file n/OutputDefaultsGen.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef N_OUTPUTDEFAULTSGEN_H_
#define N_OUTPUTDEFAULTSGEN_H_

// ----------------------------------------------------------------------------
// --------------------------- Forward Declarations ---------------------------
// ----------------------------------------------------------------------------

namespace n
{
class OutputDefaults;
} // namespace n

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <n/Grid.h>
#include <n/Pair.h>
#include <n/Single.h>
#include <Smp/IComposite.h>
#include <Smp/ISimulator.h>
#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <Xsmp/Field.h>
#include <Xsmp/Model.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace n
{
/// Universally unique identifier of type OutputDefaults.
inline constexpr ::Smp::Uuid Uuid_OutputDefaults { 0x11111111U, 0x0000U, 0x4000U, 0x8000U, 0x000000000089U };

class OutputDefaultsGen: public ::Xsmp::Model
{
    friend class ::n::OutputDefaults;
    public:
    /// Constructor setting name, description, parent and simulator.
    /// @param name Name of new Model instance.
    /// @param description Description of new Model instance.
    /// @param parent Parent of new Model instance.
    /// @param simulator The simulator instance.
    OutputDefaultsGen(::Smp::String8 name,
            ::Smp::String8 description,
            ::Smp::IComposite* parent,
            ::Smp::ISimulator* simulator);
    /// deleted copy constructor
    OutputDefaultsGen(const OutputDefaultsGen&) = delete;
    /// deleted move constructor
    OutputDefaultsGen(OutputDefaultsGen&&) = delete;
    /// deleted copy assignment
    OutputDefaultsGen& operator=(const OutputDefaultsGen&) = delete;
    /// deleted move assignment
    OutputDefaultsGen& operator=(OutputDefaultsGen&&) = delete;

    /// Virtual destructor to release memory.
    ~OutputDefaultsGen() override = default;

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
    ::Xsmp::Field<::n::Pair>::output pair;
    ::Xsmp::Field<::n::Grid>::output grid;
    ::Xsmp::Field<::n::Single>::output single;
};
} // namespace n

#endif // N_OUTPUTDEFAULTSGEN_H_
