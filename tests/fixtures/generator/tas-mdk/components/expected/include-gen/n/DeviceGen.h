// -----------------------------------------------------------------------------
// File Name    : DeviceGen.h
// -----------------------------------------------------------------------------
/// @file n/DeviceGen.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef N_DEVICEGEN_H_
#define N_DEVICEGEN_H_

// ----------------------------------------------------------------------------
// --------------------------- Forward Declarations ---------------------------
// ----------------------------------------------------------------------------

namespace n
{
class Device;
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
/// Universally unique identifier of type Device.
constexpr ::Smp::Uuid Uuid_Device { 0x11111111U, 0x0000U, 0x4000U, 0x8000U, 0x000000000144U };

class DeviceGen: public ::TasMdk::Model
{
    friend class ::n::Device;

    /// Get ready.
    /// @return Current value of property ready.
    virtual ::Smp::Bool get_ready()=0;
    /// Set ready.
    /// @param value New value of property ready to set.
    virtual void set_ready(::Smp::Bool value)=0;
    virtual void apply(::Smp::Int32 count = 42)=0;

    // ------------------------------------------------------------------------------------
    // ------------------------- Constructors/Destructor -------------------------
    // ------------------------------------------------------------------------------------
    public:
    /// Constructor setting name, description and parent.
    /// @param name Name of new model instance.
    /// @param description Description of new model instance.
    /// @param parent Parent of new model instance.
    /// @param type_registry Reference to global type registry.
    DeviceGen(
            ::Smp::String8 name,
            ::Smp::String8 description,
            ::Smp::IObject* parent,
            ::Smp::Publication::ITypeRegistry* type_registry);

    /// Virtual destructor to release memory.
    ~DeviceGen() override;

    static ::TasMdk::Request::Handler<DeviceGen>::CollectionType requestHandlers;

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
void DeviceGen::PopulateRequestHandlers(
    [[maybe_unused]] _Type* bluePrint,
    [[maybe_unused]] typename ::TasMdk::Request::Handler<_Type>::CollectionType& handlers)
{
    if (handlers.find("apply") == handlers.end()) {
        handlers["apply"] = [](_Type & component, [[maybe_unused]] ::Smp::IRequest* request) {
            ::Smp::Int32 p_count{42};
            ::TasMdk::Request::initParameter(p_count, request, "count", ::Smp::PrimitiveTypeKind::PTK_Int32, true);

            /// Invoke apply
            component.apply(p_count);
        };
    }

    if (handlers.find("get_ready") == handlers.end()) {
        handlers["get_ready"] = [](_Type & component, ::Smp::IRequest* request) {
            /// Invoke get_ready
            request->SetReturnValue({::Smp::PrimitiveTypeKind::PTK_Bool, component.get_ready()});
        };
    }

    if (handlers.find("set_ready") == handlers.end()) {
        handlers["set_ready"] = [](_Type & component, ::Smp::IRequest* request) {
            /// Invoke set_ready
            ::Smp::Bool ready;
            ::TasMdk::Request::initParameter(ready, request, "ready", ::Smp::PrimitiveTypeKind::PTK_Bool);
            component.set_ready(ready);
        };
    }
}
} // namespace n

#endif // N_DEVICEGEN_H_
