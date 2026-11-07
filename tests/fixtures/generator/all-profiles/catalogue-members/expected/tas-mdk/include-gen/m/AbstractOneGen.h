// Block notice
// line two
// -----------------------------------------------------------------------------
// File Name    : AbstractOneGen.h
// -----------------------------------------------------------------------------
/// @file m/AbstractOneGen.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef M_ABSTRACTONEGEN_H_
#define M_ABSTRACTONEGEN_H_

// ----------------------------------------------------------------------------
// --------------------------- Forward Declarations ---------------------------
// ----------------------------------------------------------------------------

namespace m
{
class AbstractOne;
} // namespace m

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <functional>
#include <m/IBase.h>
#include <Smp/ISimulator.h>
#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <type_traits>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace m
{
/// Universally unique identifier of type AbstractOne.
constexpr ::Smp::Uuid Uuid_AbstractOne { 0x20000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000020U };

/// A model.
class AbstractOneGen: public ::TasMdk::Model, public virtual ::m::IBase
{
    friend class ::m::AbstractOne;

    static constexpr ::Smp::Int32 K{2};
    /// Get connected.
    /// @return Current value of property connected.
    virtual ::Smp::Bool get_connected();
    /// Set connected.
    /// @param value New value of property connected to set.
    virtual void set_connected(::Smp::Bool value);
    /// Get ro.
    /// @return Current value of property ro.
    virtual ::Smp::Int32 get_ro();
    /// Set wo.
    /// @param value New value of property wo to set.
    virtual void set_wo(::Smp::Int32 value);
    virtual ::Smp::Int32 mustImplement()=0;
    virtual ::Smp::Bool ping(::Smp::Int32 value)=0;
    ::Smp::Bool connectedState;
    ::Smp::Int32 valueState;

    // ------------------------------------------------------------------------------------
    // ------------------------- Constructors/Destructor -------------------------
    // ------------------------------------------------------------------------------------
    public:
    /// Constructor setting name, description and parent.
    /// @param name Name of new model instance.
    /// @param description Description of new model instance.
    /// @param parent Parent of new model instance.
    /// @param type_registry Reference to global type registry.
    AbstractOneGen(
            ::Smp::String8 name,
            ::Smp::String8 description,
            ::Smp::IObject* parent,
            ::Smp::Publication::ITypeRegistry* type_registry);

    /// Virtual destructor to release memory.
    ~AbstractOneGen() override;

    static ::TasMdk::Request::Handler<AbstractOneGen>::CollectionType requestHandlers;

    // ----------------------------------------------------------------------------------
    // ------------------------------- IComponent --------------------------------
    // ----------------------------------------------------------------------------------

    /// Publish fields, operations and properties of the model.
    /// @param receiver Publication receiver.
    void Publish(::Smp::IPublication* receiver) override;

    /// Request for configuration.
    /// @param logger Logger to use for log messages during Configure().
    /// @param linkRegistry Link Registry to use for registration of
    ///         links created during Configure() or later.
    void Configure(::Smp::Services::ILogger* logger, ::Smp::Services::ILinkRegistry* linkRegistry) override;

    /// Connect model to simulator.
    /// @param simulator Simulation  Environment  that  hosts the model.
    ///
    void Connect(
        ::Smp::ISimulator* simulator) override;

    /// Disconnect model to simulator.
    /// @throws Smp::InvalidComponentState
    void Disconnect() override;

    /// Return the Universally Unique Identifier of the Model.
    /// @return  Universally Unique Identifier of the Model.
    const Smp::Uuid& GetUuid() const override;

    /// Invoke the operation for the given request.
    /// @param request Request object to invoke.
    void Invoke(::Smp::IRequest* request) override;

    protected:
    /// It populates the static map of request handler to implement the invoke function.
    /// @param bluePrint an object to use as blue print if needed
    /// @param handlers the map to be populated.
    template <typename _Type>
    static void PopulateRequestHandlers(_Type* bluePrint, typename ::TasMdk::Request::Handler<_Type>::CollectionType& handlers);

    private:
    /// Callback for custom publication
    std::function<void(::Smp::IPublication*)> Publish_Callback;
    /// Callback for custom configuration
    std::function<void(::Smp::Services::ILogger *, ::Smp::Services::ILinkRegistry*)> Configure_Callback;
    /// Callback for custom connection
    std::function<void(::Smp::ISimulator*)> Connect_Callback;
    /// Callback for custom disconnection
    std::function<void()> Disconnect_Callback;
};

template <typename _Type>
void AbstractOneGen::PopulateRequestHandlers(
    [[maybe_unused]] _Type* bluePrint,
    [[maybe_unused]] typename ::TasMdk::Request::Handler<_Type>::CollectionType& handlers)
{
    if (handlers.find("mustImplement") == handlers.end()) {
        handlers["mustImplement"] = [](_Type & component, [[maybe_unused]] ::Smp::IRequest* request) {
            /// Invoke mustImplement
            request->SetReturnValue({::Smp::PrimitiveTypeKind::PTK_Int32, component.mustImplement()});
        };
    }

    if (handlers.find("ping") == handlers.end()) {
        handlers["ping"] = [](_Type & component, [[maybe_unused]] ::Smp::IRequest* request) {
            ::Smp::Int32 p_value;
            ::TasMdk::Request::initParameter(p_value, request, "value", ::Smp::PrimitiveTypeKind::PTK_Int32);

            /// Invoke ping
            request->SetReturnValue({::Smp::PrimitiveTypeKind::PTK_Bool, component.ping(p_value)});
        };
    }

    if (handlers.find("get_connected") == handlers.end()) {
        handlers["get_connected"] = [](_Type & component, ::Smp::IRequest* request) {
            /// Invoke get_connected
            request->SetReturnValue({::Smp::PrimitiveTypeKind::PTK_Bool, component.get_connected()});
        };
    }

    if (handlers.find("set_connected") == handlers.end()) {
        handlers["set_connected"] = [](_Type & component, ::Smp::IRequest* request) {
            /// Invoke set_connected
            ::Smp::Bool connected;
            ::TasMdk::Request::initParameter(connected, request, "connected", ::Smp::PrimitiveTypeKind::PTK_Bool);
            component.set_connected(connected);
        };
    }

    if (handlers.find("get_ro") == handlers.end()) {
        handlers["get_ro"] = [](_Type & component, ::Smp::IRequest* request) {
            /// Invoke get_ro
            request->SetReturnValue({::Smp::PrimitiveTypeKind::PTK_Int32, component.get_ro()});
        };
    }

    if (handlers.find("set_wo") == handlers.end()) {
        handlers["set_wo"] = [](_Type & component, ::Smp::IRequest* request) {
            /// Invoke set_wo
            ::Smp::Int32 wo;
            ::TasMdk::Request::initParameter(wo, request, "wo", ::Smp::PrimitiveTypeKind::PTK_Int32);
            component.set_wo(wo);
        };
    }
}
} // namespace m

#endif // M_ABSTRACTONEGEN_H_
