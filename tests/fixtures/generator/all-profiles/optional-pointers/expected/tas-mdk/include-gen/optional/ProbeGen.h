// -----------------------------------------------------------------------------
// File Name    : ProbeGen.h
// -----------------------------------------------------------------------------
/// @file optional/ProbeGen.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef OPTIONAL_PROBEGEN_H_
#define OPTIONAL_PROBEGEN_H_

// ----------------------------------------------------------------------------
// --------------------------- Forward Declarations ---------------------------
// ----------------------------------------------------------------------------

namespace optional
{
class Probe;
} // namespace optional

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

namespace optional
{
/// Universally unique identifier of type Probe.
constexpr ::Smp::Uuid Uuid_Probe { 0x86000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000001U };

class ProbeGen: public ::TasMdk::Model
{
    friend class ::optional::Probe;

    /// Optional input.
    virtual void ope_input(::Smp::Int32* value = nullptr)=0;
    /// The request owns the String8 until the handler returns.
    virtual void ope_string(::Smp::String8* value = nullptr)=0;
    /// Optional output.
    virtual void ope_output(::Smp::Int32* value = nullptr)=0;
    /// Optional input/output.
    virtual void ope_inout(::Smp::Int32* value = nullptr)=0;
    /// The parameter names must not collide with generated presence flags.
    virtual void ope_names(::Smp::Int32* value = nullptr, ::Smp::Int32* value_present = nullptr)=0;

    // ------------------------------------------------------------------------------------
    // ------------------------- Constructors/Destructor -------------------------
    // ------------------------------------------------------------------------------------
    public:
    /// Constructor setting name, description and parent.
    /// @param name Name of new model instance.
    /// @param description Description of new model instance.
    /// @param parent Parent of new model instance.
    /// @param type_registry Reference to global type registry.
    ProbeGen(
            ::Smp::String8 name,
            ::Smp::String8 description,
            ::Smp::IObject* parent,
            ::Smp::Publication::ITypeRegistry* type_registry);

    /// Virtual destructor to release memory.
    ~ProbeGen() override;

    static ::TasMdk::Request::Handler<ProbeGen>::CollectionType requestHandlers;

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
void ProbeGen::PopulateRequestHandlers(
    [[maybe_unused]] _Type* bluePrint,
    [[maybe_unused]] typename ::TasMdk::Request::Handler<_Type>::CollectionType& handlers)
{
    if (handlers.find("ope_input") == handlers.end()) {
        handlers["ope_input"] = [](_Type & component, [[maybe_unused]] ::Smp::IRequest* request) {
            bool has_value = request->GetParameterIndex("value") >= 0;
            ::Smp::Int32 p_value{};
            ::Smp::AnySimple storage_value;
            if (has_value) {
                ::TasMdk::Request::initParameter(storage_value, request, "value", ::Smp::PrimitiveTypeKind::PTK_Int32, true);
                has_value = storage_value.GetType() != ::Smp::PrimitiveTypeKind::PTK_None;
                if (has_value) { p_value = static_cast<::Smp::Int32>(storage_value); }
            }

            /// Invoke ope_input
            component.ope_input(has_value ? &p_value : nullptr);
        };
    }

    if (handlers.find("ope_string") == handlers.end()) {
        handlers["ope_string"] = [](_Type & component, [[maybe_unused]] ::Smp::IRequest* request) {
            bool has_value = request->GetParameterIndex("value") >= 0;
            ::Smp::String8 p_value{};
            ::Smp::AnySimple storage_value;
            if (has_value) {
                ::TasMdk::Request::initParameter(storage_value, request, "value", ::Smp::PrimitiveTypeKind::PTK_String8, true);
                has_value = storage_value.GetType() != ::Smp::PrimitiveTypeKind::PTK_None;
                if (has_value) { p_value = static_cast<::Smp::String8>(storage_value); }
            }

            /// Invoke ope_string
            component.ope_string(has_value ? &p_value : nullptr);
        };
    }

    if (handlers.find("ope_output") == handlers.end()) {
        handlers["ope_output"] = [](_Type & component, [[maybe_unused]] ::Smp::IRequest* request) {
            const bool has_value = request->GetParameterIndex("value") >= 0;
            ::Smp::Int32 p_value{};

            /// Invoke ope_output
            component.ope_output(has_value ? &p_value : nullptr);
            if (has_value) { ::TasMdk::Request::setParameter(request, "value", {::Smp::PrimitiveTypeKind::PTK_Int32, p_value}); }
        };
    }

    if (handlers.find("ope_inout") == handlers.end()) {
        handlers["ope_inout"] = [](_Type & component, [[maybe_unused]] ::Smp::IRequest* request) {
            bool has_value = request->GetParameterIndex("value") >= 0;
            ::Smp::Int32 p_value{};
            ::Smp::AnySimple storage_value;
            if (has_value) {
                ::TasMdk::Request::initParameter(storage_value, request, "value", ::Smp::PrimitiveTypeKind::PTK_Int32, true);
                has_value = storage_value.GetType() != ::Smp::PrimitiveTypeKind::PTK_None;
                if (has_value) { p_value = static_cast<::Smp::Int32>(storage_value); }
            }

            /// Invoke ope_inout
            component.ope_inout(has_value ? &p_value : nullptr);
            if (has_value) { ::TasMdk::Request::setParameter(request, "value", {::Smp::PrimitiveTypeKind::PTK_Int32, p_value}); }
        };
    }

    if (handlers.find("ope_names") == handlers.end()) {
        handlers["ope_names"] = [](_Type & component, [[maybe_unused]] ::Smp::IRequest* request) {
            bool has_value = request->GetParameterIndex("value") >= 0;
            ::Smp::Int32 p_value{};
            ::Smp::AnySimple storage_value;
            if (has_value) {
                ::TasMdk::Request::initParameter(storage_value, request, "value", ::Smp::PrimitiveTypeKind::PTK_Int32, true);
                has_value = storage_value.GetType() != ::Smp::PrimitiveTypeKind::PTK_None;
                if (has_value) { p_value = static_cast<::Smp::Int32>(storage_value); }
            }
            bool has_value_present = request->GetParameterIndex("value_present") >= 0;
            ::Smp::Int32 p_value_present{};
            ::Smp::AnySimple storage_value_present;
            if (has_value_present) {
                ::TasMdk::Request::initParameter(storage_value_present, request, "value_present", ::Smp::PrimitiveTypeKind::PTK_Int32, true);
                has_value_present = storage_value_present.GetType() != ::Smp::PrimitiveTypeKind::PTK_None;
                if (has_value_present) { p_value_present = static_cast<::Smp::Int32>(storage_value_present); }
            }

            /// Invoke ope_names
            component.ope_names(has_value ? &p_value : nullptr, has_value_present ? &p_value_present : nullptr);
        };
    }
}
} // namespace optional

#endif // OPTIONAL_PROBEGEN_H_
