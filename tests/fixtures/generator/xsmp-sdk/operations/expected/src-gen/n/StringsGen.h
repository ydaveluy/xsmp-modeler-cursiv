// -----------------------------------------------------------------------------
// File Name    : StringsGen.h
// -----------------------------------------------------------------------------
/// @file n/StringsGen.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef N_STRINGSGEN_H_
#define N_STRINGSGEN_H_

// ----------------------------------------------------------------------------
// --------------------------- Forward Declarations ---------------------------
// ----------------------------------------------------------------------------

namespace n
{
class Strings;
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
/// Universally unique identifier of type Strings.
inline constexpr ::Smp::Uuid Uuid_Strings { 0x11111111U, 0x0000U, 0x4000U, 0x8000U, 0x000000000074U };

class StringsGen: public ::Xsmp::Model
{
    friend class ::n::Strings;
    public:
    /// Constructor setting name, description, parent and simulator.
    /// @param name Name of new Model instance.
    /// @param description Description of new Model instance.
    /// @param parent Parent of new Model instance.
    /// @param simulator The simulator instance.
    StringsGen(::Smp::String8 name,
            ::Smp::String8 description,
            ::Smp::IComposite* parent,
            ::Smp::ISimulator* simulator);
    /// deleted copy constructor
    StringsGen(const StringsGen&) = delete;
    /// deleted move constructor
    StringsGen(StringsGen&&) = delete;
    /// deleted copy assignment
    StringsGen& operator=(const StringsGen&) = delete;
    /// deleted move assignment
    StringsGen& operator=(StringsGen&&) = delete;

    /// Virtual destructor to release memory.
    ~StringsGen() override = default;

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
    static std::map<std::string_view, std::function<void(StringsGen*, ::Smp::IRequest*)>> _requestHandlers;

    public:
    /// Dynamically invoke an operation using a request object that has
    /// been created and filled with parameter values by the caller.
    /// @param   request Request object to invoke.
    /// @throws  Smp::InvalidOperationName
    /// @throws  Smp::InvalidParameterCount
    /// @throws  Smp::InvalidParameterType
    void Invoke(::Smp::IRequest* request) override;

    private:
    /// Get enabled.
    /// @return Current value of property enabled.
    virtual ::Smp::Bool get_enabled()=0;
    /// Set enabled.
    /// @param value New value of property enabled to set.
    virtual void set_enabled(::Smp::Bool value)=0;
    virtual void inputString(::Smp::String8 value)=0;
    virtual void outputString(::Smp::String8* value)=0;
    virtual void inoutString(::Smp::String8* value)=0;
    virtual ::Smp::String8 returnString()=0;
    virtual ::Smp::Bool returnBool()=0;
};
} // namespace n

#endif // N_STRINGSGEN_H_
