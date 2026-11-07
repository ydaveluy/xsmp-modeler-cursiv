// -----------------------------------------------------------------------------
// File Name    : ProbeGen.h
// -----------------------------------------------------------------------------
/// @file optional/ProbeGen.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef OPTIONAL_PROBEGEN_H_
#define OPTIONAL_PROBEGEN_H_

// ----------------------------------------------------------------------------
// --------------------------- Forward Declarations ---------------------------
// ----------------------------------------------------------------------------

namespace optional
{
class Probe;
} // namespace optional

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <functional>
#include <map>
#include <Smp/IComposite.h>
#include <Smp/IRequest.h>
#include <Smp/ISimulator.h>
#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <string_view>
#include <Xsmp/Model.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace optional
{
/// Universally unique identifier of type Probe.
inline constexpr ::Smp::Uuid Uuid_Probe { 0x86000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000001U };

class ProbeGen: public ::Xsmp::Model
{
    friend class ::optional::Probe;
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
    ~ProbeGen() override = default;

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
    static std::map<std::string_view, std::function<void(ProbeGen*, ::Smp::IRequest*)>> _requestHandlers;

    public:
    /// Dynamically invoke an operation using a request object that has
    /// been created and filled with parameter values by the caller.
    /// @param   request Request object to invoke.
    /// @throws  Smp::InvalidOperationName
    /// @throws  Smp::InvalidParameterCount
    /// @throws  Smp::InvalidParameterType
    void Invoke(::Smp::IRequest* request) override;

    private:
    /// Optional input.
    virtual void ope_input(::Smp::Int32* value = nullptr)=0;
    /// The request owns the String8 until the handler returns.
    virtual void ope_string(::Smp::String8* value = nullptr)=0;
    /// Optional output.
    virtual void ope_output(::Smp::Int32* value = nullptr)=0;
    /// Optional input/output.
    virtual void ope_inout(::Smp::Int32* value = nullptr)=0;
    /// The parameter names must not collide with generated presence flags.
    virtual void ope_names(::Smp::Int32* value = nullptr, ::Smp::Int32* value_present = nullptr)=0;
};
} // namespace optional

#endif // OPTIONAL_PROBEGEN_H_
