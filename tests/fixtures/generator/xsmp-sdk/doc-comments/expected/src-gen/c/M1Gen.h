// -----------------------------------------------------------------------------
// File Name    : M1Gen.h
// -----------------------------------------------------------------------------
/// @file c/M1Gen.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef C_M1GEN_H_
#define C_M1GEN_H_

// ----------------------------------------------------------------------------
// --------------------------- Forward Declarations ---------------------------
// ----------------------------------------------------------------------------

namespace c
{
class M1;
} // namespace c

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

namespace c
{
/// Universally unique identifier of type M1.
inline constexpr ::Smp::Uuid Uuid_M1 { 0x30000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000003U };

/// See {@link T1} inline and {@link T2 two}.
class M1Gen: public ::Xsmp::Model
{
    friend class ::c::M1;
    public:
    /// Constructor setting name, description, parent and simulator.
    /// @param name Name of new Model instance.
    /// @param description Description of new Model instance.
    /// @param parent Parent of new Model instance.
    /// @param simulator The simulator instance.
    M1Gen(::Smp::String8 name,
            ::Smp::String8 description,
            ::Smp::IComposite* parent,
            ::Smp::ISimulator* simulator);
    /// deleted copy constructor
    M1Gen(const M1Gen&) = delete;
    /// deleted move constructor
    M1Gen(M1Gen&&) = delete;
    /// deleted copy assignment
    M1Gen& operator=(const M1Gen&) = delete;
    /// deleted move assignment
    M1Gen& operator=(M1Gen&&) = delete;

    /// Virtual destructor to release memory.
    ~M1Gen() override = default;

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
    static std::map<std::string_view, std::function<void(M1Gen*, ::Smp::IRequest*)>> _requestHandlers;

    public:
    /// Dynamically invoke an operation using a request object that has
    /// been created and filled with parameter values by the caller.
    /// @param   request Request object to invoke.
    /// @throws  Smp::InvalidOperationName
    /// @throws  Smp::InvalidParameterCount
    /// @throws  Smp::InvalidParameterType
    void Invoke(::Smp::IRequest* request) override;

    private:
    /// Does things.
    /// @param a The first, see {@link T1}.
    /// @param b The second
    ///          on two lines.
    /// @param c   Spaced.
    /// @return The result, it's fine.
    /// @throws c.Oops never
    virtual ::Smp::Int32 f(::Smp::Int32 a, ::Smp::Int32 b, ::Smp::Int32 c)=0;
    /// @param x only a tag
    virtual void noText(::Smp::Int32 x)=0;
    /// Trailing backslash &#92;
    /// and next line
    ::Smp::Int32 g;
    /// With a tab	inside and unicode é ü 漢字.
    ::Smp::Int32 h;
    /// A question ??= trigraph and tab	.
    ::Smp::Int32 i;
    ::Smp::Int32 j;
    ::Smp::Int32 k;
    ::Smp::Int32 l;
    ::Smp::Int32 n;
    /// {@uuid inline} kept
    ::Smp::Int32 o;
    /// Trailing spaces
    /// end
    ::Smp::Int32 p;
};
} // namespace c

#endif // C_M1GEN_H_
