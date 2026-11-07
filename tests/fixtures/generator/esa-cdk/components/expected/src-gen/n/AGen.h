// -----------------------------------------------------------------------------
// File Name    : AGen.h
// -----------------------------------------------------------------------------
/// @file n/AGen.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef N_AGEN_H_
#define N_AGEN_H_

// ----------------------------------------------------------------------------
// --------------------------- Forward Declarations ---------------------------
// ----------------------------------------------------------------------------

namespace n
{
class A;
} // namespace n

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <esa/ecss/smp/cdk/Model.h>
#include <esa/ecss/smp/cdk/Request.h>
#include <esa/ecss/smp/cdk/RequestContainer.h>
#include <map>
#include <Smp/IComposite.h>
#include <Smp/IRequest.h>
#include <Smp/ISimulator.h>
#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/ITypeRegistry.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace n
{
/// Universally unique identifier of type A.
constexpr ::Smp::Uuid Uuid_A { 0x11111111U, 0x0000U, 0x4000U, 0x8000U, 0x000000000122U };

class AGen: public ::esa::ecss::smp::cdk::Model
{
    friend class ::n::A;
    public:
    /// Constructor setting name, description, parent and simulator.
    /// @param name Name of new Model instance.
    /// @param description Description of new Model instance.
    /// @param parent Parent of new Model instance.
    /// @param simulator The simulator instance.
    AGen(::Smp::String8 name,
            ::Smp::String8 description,
            ::Smp::IComposite* parent,
            ::Smp::ISimulator* simulator);
    /// deleted copy constructor
    AGen(const AGen&) = delete;
    /// deleted move constructor
    AGen(AGen&&) = delete;
    /// deleted copy assignment
    AGen& operator=(const AGen&) = delete;
    /// deleted move assignment
    AGen& operator=(AGen&&) = delete;

    /// Virtual destructor to release memory.
    ~AGen() override;

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
    template <typename _Type> static void PopulateRequestHandlers(_Type* bluePrint, typename ::esa::ecss::smp::cdk::RequestContainer<_Type>::Map& handlers);
    static ::esa::ecss::smp::cdk::RequestContainer<AGen>::Map requestHandlers;

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

template <typename _Type>
void AGen::PopulateRequestHandlers(_Type* bluePrint, typename ::esa::ecss::smp::cdk::RequestContainer<_Type>::Map& handlers)
{
    typedef ::esa::ecss::smp::cdk::RequestContainer<_Type> Help;

    // ---- Operations ----
    Help::template AddIfMissing<void>(
        handlers,
        "f",
        ::Smp::PrimitiveTypeKind::PTK_None,
        static_cast<void (AGen::*)()>(&AGen::f));

    ::esa::ecss::smp::cdk::Model::PopulateRequestHandlers<_Type>(bluePrint, handlers);
}
} // namespace n

#endif // N_AGEN_H_
