// -----------------------------------------------------------------------------
// File Name    : NullDefaultsGen.h
// -----------------------------------------------------------------------------
/// @file n/NullDefaultsGen.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef N_NULLDEFAULTSGEN_H_
#define N_NULLDEFAULTSGEN_H_

// ----------------------------------------------------------------------------
// --------------------------- Forward Declarations ---------------------------
// ----------------------------------------------------------------------------

namespace n
{
class NullDefaults;
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
/// Universally unique identifier of type NullDefaults.
constexpr ::Smp::Uuid Uuid_NullDefaults { 0x11111111U, 0x0000U, 0x4000U, 0x8000U, 0x000000000146U };

class NullDefaultsGen: public ::TasMdk::Model
{
    friend class ::n::NullDefaults;

    virtual void ping(::Smp::Int32* a = nullptr, ::Smp::Int32 b = 2, ::Smp::Int32* c = nullptr, ::Smp::Char8 d = '\0')=0;

    // ------------------------------------------------------------------------------------
    // ------------------------- Constructors/Destructor -------------------------
    // ------------------------------------------------------------------------------------
    public:
    /// Constructor setting name, description and parent.
    /// @param name Name of new model instance.
    /// @param description Description of new model instance.
    /// @param parent Parent of new model instance.
    /// @param type_registry Reference to global type registry.
    NullDefaultsGen(
            ::Smp::String8 name,
            ::Smp::String8 description,
            ::Smp::IObject* parent,
            ::Smp::Publication::ITypeRegistry* type_registry);

    /// Virtual destructor to release memory.
    ~NullDefaultsGen() override;

    static ::TasMdk::Request::Handler<NullDefaultsGen>::CollectionType requestHandlers;

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
void NullDefaultsGen::PopulateRequestHandlers(
    [[maybe_unused]] _Type* bluePrint,
    [[maybe_unused]] typename ::TasMdk::Request::Handler<_Type>::CollectionType& handlers)
{
    if (handlers.find("ping") == handlers.end()) {
        handlers["ping"] = [](_Type & component, [[maybe_unused]] ::Smp::IRequest* request) {
            bool has_a = request->GetParameterIndex("a") >= 0;
            ::Smp::Int32 p_a{};
            ::Smp::AnySimple storage_a;
            if (has_a) {
                ::TasMdk::Request::initParameter(storage_a, request, "a", ::Smp::PrimitiveTypeKind::PTK_Int32, true);
                has_a = storage_a.GetType() != ::Smp::PrimitiveTypeKind::PTK_None;
                if (has_a) { p_a = static_cast<::Smp::Int32>(storage_a); }
            }
            ::Smp::Int32 p_b{2};
            ::TasMdk::Request::initParameter(p_b, request, "b", ::Smp::PrimitiveTypeKind::PTK_Int32, true);
            bool has_c = request->GetParameterIndex("c") >= 0;
            ::Smp::Int32 p_c{};
            ::Smp::AnySimple storage_c;
            if (has_c) {
                ::TasMdk::Request::initParameter(storage_c, request, "c", ::Smp::PrimitiveTypeKind::PTK_Int32, true);
                has_c = storage_c.GetType() != ::Smp::PrimitiveTypeKind::PTK_None;
                if (has_c) { p_c = static_cast<::Smp::Int32>(storage_c); }
            }
            ::Smp::Char8 p_d{'\0'};
            ::TasMdk::Request::initParameter(p_d, request, "d", ::Smp::PrimitiveTypeKind::PTK_Char8, true);

            /// Invoke ping
            component.ping(has_a ? &p_a : nullptr, p_b, has_c ? &p_c : nullptr, p_d);
            if (has_a) { ::TasMdk::Request::setParameter(request, "a", {::Smp::PrimitiveTypeKind::PTK_Int32, p_a}); }
        };
    }
}
} // namespace n

#endif // N_NULLDEFAULTSGEN_H_
