// -----------------------------------------------------------------------------
// File Name    : SGen.h
// -----------------------------------------------------------------------------
/// @file n/SGen.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef N_SGEN_H_
#define N_SGEN_H_

// ----------------------------------------------------------------------------
// --------------------------- Forward Declarations ---------------------------
// ----------------------------------------------------------------------------

namespace n
{
class S;
} // namespace n

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <esa/ecss/smp/cdk/Request.h>
#include <esa/ecss/smp/cdk/RequestContainer.h>
#include <esa/ecss/smp/cdk/Service.h>
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
/// Universally unique identifier of type S.
constexpr ::Smp::Uuid Uuid_S { 0x11111111U, 0x0000U, 0x4000U, 0x8000U, 0x000000000124U };

class SGen: public ::esa::ecss::smp::cdk::Service
{
    friend class ::n::S;
    public:
    /// Constructor setting name, description, parent and simulator.
    /// @param name Name of new Service instance.
    /// @param description Description of new Service instance.
    /// @param parent Parent of new Service instance.
    /// @param simulator The simulator instance.
    SGen(::Smp::String8 name,
            ::Smp::String8 description,
            ::Smp::IComposite* parent,
            ::Smp::ISimulator* simulator);
    /// deleted copy constructor
    SGen(const SGen&) = delete;
    /// deleted move constructor
    SGen(SGen&&) = delete;
    /// deleted copy assignment
    SGen& operator=(const SGen&) = delete;
    /// deleted move assignment
    SGen& operator=(SGen&&) = delete;

    /// Virtual destructor to release memory.
    ~SGen() override;

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
    private:
    template <typename _Type> static void PopulateRequestHandlers(_Type* bluePrint, typename ::esa::ecss::smp::cdk::RequestContainer<_Type>::Map& handlers);
    static ::esa::ecss::smp::cdk::RequestContainer<SGen>::Map requestHandlers;

    public:
    /// Dynamically invoke an operation using a request object that has
    /// been created and filled with parameter values by the caller.
    /// @param   request Request object to invoke.
    /// @throws  Smp::InvalidOperationName
    /// @throws  Smp::InvalidParameterCount
    /// @throws  Smp::InvalidParameterType
    void Invoke(::Smp::IRequest* request) override;

    private:
    virtual void h()=0;
};

template <typename _Type>
void SGen::PopulateRequestHandlers(_Type* bluePrint, typename ::esa::ecss::smp::cdk::RequestContainer<_Type>::Map& handlers)
{
    typedef ::esa::ecss::smp::cdk::RequestContainer<_Type> Help;

    // ---- Operations ----
    Help::template AddIfMissing<void>(
        handlers,
        "h",
        ::Smp::PrimitiveTypeKind::PTK_None,
        static_cast<void (SGen::*)()>(&SGen::h));

    ::esa::ecss::smp::cdk::Service::PopulateRequestHandlers<_Type>(bluePrint, handlers);
}
} // namespace n

#endif // N_SGEN_H_
