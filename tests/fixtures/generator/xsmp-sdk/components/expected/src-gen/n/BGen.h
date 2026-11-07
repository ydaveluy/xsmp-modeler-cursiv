// -----------------------------------------------------------------------------
// File Name    : BGen.h
// -----------------------------------------------------------------------------
/// @file n/BGen.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef N_BGEN_H_
#define N_BGEN_H_

// ----------------------------------------------------------------------------
// --------------------------- Forward Declarations ---------------------------
// ----------------------------------------------------------------------------

namespace n
{
class B;
} // namespace n

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <functional>
#include <map>
#include <n/A.h>
#include <Smp/IComposite.h>
#include <Smp/IRequest.h>
#include <Smp/ISimulator.h>
#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <string_view>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace n
{
/// Universally unique identifier of type B.
inline constexpr ::Smp::Uuid Uuid_B { 0x11111111U, 0x0000U, 0x4000U, 0x8000U, 0x000000000068U };

class BGen: public ::n::A
{
    friend class ::n::B;
    public:
    /// Constructor setting name, description, parent and simulator.
    /// @param name Name of new Model instance.
    /// @param description Description of new Model instance.
    /// @param parent Parent of new Model instance.
    /// @param simulator The simulator instance.
    BGen(::Smp::String8 name,
            ::Smp::String8 description,
            ::Smp::IComposite* parent,
            ::Smp::ISimulator* simulator);
    /// deleted copy constructor
    BGen(const BGen&) = delete;
    /// deleted move constructor
    BGen(BGen&&) = delete;
    /// deleted copy assignment
    BGen& operator=(const BGen&) = delete;
    /// deleted move assignment
    BGen& operator=(BGen&&) = delete;

    /// Virtual destructor to release memory.
    ~BGen() override = default;

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
    static std::map<std::string_view, std::function<void(BGen*, ::Smp::IRequest*)>> _requestHandlers;

    public:
    /// Dynamically invoke an operation using a request object that has
    /// been created and filled with parameter values by the caller.
    /// @param   request Request object to invoke.
    /// @throws  Smp::InvalidOperationName
    /// @throws  Smp::InvalidParameterCount
    /// @throws  Smp::InvalidParameterType
    void Invoke(::Smp::IRequest* request) override;

    private:
    virtual void f()=0;
};
} // namespace n

#endif // N_BGEN_H_
