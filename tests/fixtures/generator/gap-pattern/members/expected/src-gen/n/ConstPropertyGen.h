// -----------------------------------------------------------------------------
// File Name    : ConstPropertyGen.h
// -----------------------------------------------------------------------------
/// @file n/ConstPropertyGen.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef N_CONSTPROPERTYGEN_H_
#define N_CONSTPROPERTYGEN_H_

// ----------------------------------------------------------------------------
// --------------------------- Forward Declarations ---------------------------
// ----------------------------------------------------------------------------

namespace n
{
class ConstProperty;
} // namespace n

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

namespace n
{
/// Universally unique identifier of type ConstProperty.
inline constexpr ::Smp::Uuid Uuid_ConstProperty { 0x11111111U, 0x0000U, 0x4000U, 0x8000U, 0x0000000000b3U };

class ConstPropertyGen: public ::Xsmp::Model
{
    friend class ::n::ConstProperty;
    public:
    /// Constructor setting name, description, parent and simulator.
    /// @param name Name of new Model instance.
    /// @param description Description of new Model instance.
    /// @param parent Parent of new Model instance.
    /// @param simulator The simulator instance.
    ConstPropertyGen(::Smp::String8 name,
            ::Smp::String8 description,
            ::Smp::IComposite* parent,
            ::Smp::ISimulator* simulator);
    /// deleted copy constructor
    ConstPropertyGen(const ConstPropertyGen&) = delete;
    /// deleted move constructor
    ConstPropertyGen(ConstPropertyGen&&) = delete;
    /// deleted copy assignment
    ConstPropertyGen& operator=(const ConstPropertyGen&) = delete;
    /// deleted move assignment
    ConstPropertyGen& operator=(ConstPropertyGen&&) = delete;

    /// Virtual destructor to release memory.
    ~ConstPropertyGen() override = default;

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
    static std::map<std::string_view, std::function<void(ConstPropertyGen*, ::Smp::IRequest*)>> _requestHandlers;

    public:
    /// Dynamically invoke an operation using a request object that has
    /// been created and filled with parameter values by the caller.
    /// @param   request Request object to invoke.
    /// @throws  Smp::InvalidOperationName
    /// @throws  Smp::InvalidParameterCount
    /// @throws  Smp::InvalidParameterType
    void Invoke(::Smp::IRequest* request) override;

    private:
    /// Get p.
    /// @return Current value of property p.
    virtual const ::Smp::Int32 get_p()=0;
    /// Set p.
    /// @param value New value of property p to set.
    virtual void set_p(const ::Smp::Int32 value)=0;
};
} // namespace n

#endif // N_CONSTPROPERTYGEN_H_
