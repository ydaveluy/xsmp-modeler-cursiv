// -----------------------------------------------------------------------------
// File Name    : InvokableGen.h
// -----------------------------------------------------------------------------
/// @file n/InvokableGen.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef N_INVOKABLEGEN_H_
#define N_INVOKABLEGEN_H_

// ----------------------------------------------------------------------------
// --------------------------- Forward Declarations ---------------------------
// ----------------------------------------------------------------------------

namespace n
{
class Invokable;
} // namespace n

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <functional>
#include <Smp/ISimulator.h>
#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <type_traits>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace n
{
/// Universally unique identifier of type Invokable.
constexpr ::Smp::Uuid Uuid_Invokable { 0x11111111U, 0x0000U, 0x4000U, 0x8000U, 0x000000000145U };

class InvokableGen: public ::TasMdk::Model
{
    friend class ::n::Invokable;

    /// Get pointer.
    /// @return Current value of property pointer.
    virtual ::Smp::Int32* get_pointer()=0;
    /// Set pointer.
    /// @param value New value of property pointer to set.
    virtual void set_pointer(::Smp::Int32* value)=0;
    /// Get plain.
    /// @return Current value of property plain.
    virtual ::Smp::Int32 get_plain()=0;
    /// Set plain.
    /// @param value New value of property plain to set.
    virtual void set_plain(::Smp::Int32 value)=0;
    virtual void ping(::Smp::Int32 count)=0;
    InvokableGen(::Smp::Int32 a);

    // ------------------------------------------------------------------------------------
    // ------------------------- Constructors/Destructor -------------------------
    // ------------------------------------------------------------------------------------
    public:
    /// Constructor setting name, description and parent.
    /// @param name Name of new model instance.
    /// @param description Description of new model instance.
    /// @param parent Parent of new model instance.
    /// @param type_registry Reference to global type registry.
    InvokableGen(
            ::Smp::String8 name,
            ::Smp::String8 description,
            ::Smp::IObject* parent,
            ::Smp::Publication::ITypeRegistry* type_registry);

    /// Virtual destructor to release memory.
    ~InvokableGen() override;

    static ::TasMdk::Request::Handler<InvokableGen>::CollectionType requestHandlers;

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
void InvokableGen::PopulateRequestHandlers(
    [[maybe_unused]] _Type* bluePrint,
    [[maybe_unused]] typename ::TasMdk::Request::Handler<_Type>::CollectionType& handlers)
{
    if (handlers.find("ping") == handlers.end()) {
        handlers["ping"] = [](_Type & component, [[maybe_unused]] ::Smp::IRequest* request) {
            ::Smp::Int32 p_count;
            ::TasMdk::Request::initParameter(p_count, request, "count", ::Smp::PrimitiveTypeKind::PTK_Int32);

            /// Invoke ping
            component.ping(p_count);
        };
    }

    if (handlers.find("get_plain") == handlers.end()) {
        handlers["get_plain"] = [](_Type & component, ::Smp::IRequest* request) {
            /// Invoke get_plain
            request->SetReturnValue({::Smp::PrimitiveTypeKind::PTK_Int32, component.get_plain()});
        };
    }

    if (handlers.find("set_plain") == handlers.end()) {
        handlers["set_plain"] = [](_Type & component, ::Smp::IRequest* request) {
            /// Invoke set_plain
            ::Smp::Int32 plain;
            ::TasMdk::Request::initParameter(plain, request, "plain", ::Smp::PrimitiveTypeKind::PTK_Int32);
            component.set_plain(plain);
        };
    }
}
} // namespace n

#endif // N_INVOKABLEGEN_H_
