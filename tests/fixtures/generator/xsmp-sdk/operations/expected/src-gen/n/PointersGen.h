// -----------------------------------------------------------------------------
// File Name    : PointersGen.h
// -----------------------------------------------------------------------------
/// @file n/PointersGen.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef N_POINTERSGEN_H_
#define N_POINTERSGEN_H_

// ----------------------------------------------------------------------------
// --------------------------- Forward Declarations ---------------------------
// ----------------------------------------------------------------------------

namespace n
{
class Pointers;
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
/// Universally unique identifier of type Pointers.
inline constexpr ::Smp::Uuid Uuid_Pointers { 0x11111111U, 0x0000U, 0x4000U, 0x8000U, 0x00000000007aU };

class PointersGen: public ::Xsmp::Model
{
    friend class ::n::Pointers;
    public:
    /// Constructor setting name, description, parent and simulator.
    /// @param name Name of new Model instance.
    /// @param description Description of new Model instance.
    /// @param parent Parent of new Model instance.
    /// @param simulator The simulator instance.
    PointersGen(::Smp::String8 name,
            ::Smp::String8 description,
            ::Smp::IComposite* parent,
            ::Smp::ISimulator* simulator);
    /// deleted copy constructor
    PointersGen(const PointersGen&) = delete;
    /// deleted move constructor
    PointersGen(PointersGen&&) = delete;
    /// deleted copy assignment
    PointersGen& operator=(const PointersGen&) = delete;
    /// deleted move assignment
    PointersGen& operator=(PointersGen&&) = delete;

    /// Virtual destructor to release memory.
    ~PointersGen() override = default;

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
    static std::map<std::string_view, std::function<void(PointersGen*, ::Smp::IRequest*)>> _requestHandlers;

    public:
    /// Dynamically invoke an operation using a request object that has
    /// been created and filled with parameter values by the caller.
    /// @param   request Request object to invoke.
    /// @throws  Smp::InvalidOperationName
    /// @throws  Smp::InvalidParameterCount
    /// @throws  Smp::InvalidParameterType
    void Invoke(::Smp::IRequest* request) override;

    private:
    /// Get pointer.
    /// @return Current value of property pointer.
    virtual ::Smp::Int32* get_pointer()=0;
    /// Set pointer.
    /// @param value New value of property pointer to set.
    virtual void set_pointer(::Smp::Int32* value)=0;
    /// Get reference.
    /// @return Current value of property reference.
    virtual ::Smp::Int32& get_reference()=0;
    /// Set reference.
    /// @param value New value of property reference to set.
    virtual void set_reference(::Smp::Int32& value)=0;
    /// Get plain.
    /// @return Current value of property plain.
    virtual ::Smp::Int32 get_plain()=0;
    /// Set plain.
    /// @param value New value of property plain to set.
    virtual void set_plain(::Smp::Int32 value)=0;
    virtual ::Smp::Int32* address()=0;
    virtual ::Smp::Int32& alias()=0;
};
} // namespace n

#endif // N_POINTERSGEN_H_
