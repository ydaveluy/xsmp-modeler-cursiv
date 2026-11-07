// -----------------------------------------------------------------------------
// File Name    : MGen.h
// -----------------------------------------------------------------------------
/// @file n/MGen.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef N_MGEN_H_
#define N_MGEN_H_

// ----------------------------------------------------------------------------
// --------------------------- Forward Declarations ---------------------------
// ----------------------------------------------------------------------------

namespace n
{
class M;
} // namespace n

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <functional>
#include <Smp/IEntryPoint.h>
#include <Smp/ISimulator.h>
#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <TasMdk/Field.h>
#include <type_traits>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace n
{
/// Universally unique identifier of type M.
constexpr ::Smp::Uuid Uuid_M { 0x11111111U, 0x0000U, 0x4000U, 0x8000U, 0x000000000142U };

class MGen: public ::TasMdk::Model
{
    friend class ::n::M;

    virtual void ope_reset()=0;
    public:
    ::Smp::IEntryPoint* ept_step;
    virtual void _ept_step() = 0;
    private:
    typename ::TasMdk::SimpleField<::Smp::Int32>::input::type inp_value;

    // ------------------------------------------------------------------------------------
    // ------------------------- Constructors/Destructor -------------------------
    // ------------------------------------------------------------------------------------
    public:
    /// Constructor setting name, description and parent.
    /// @param name Name of new model instance.
    /// @param description Description of new model instance.
    /// @param parent Parent of new model instance.
    /// @param type_registry Reference to global type registry.
    MGen(
            ::Smp::String8 name,
            ::Smp::String8 description,
            ::Smp::IObject* parent,
            ::Smp::Publication::ITypeRegistry* type_registry);

    /// Virtual destructor to release memory.
    ~MGen() override;

    static ::TasMdk::Request::Handler<MGen>::CollectionType requestHandlers;

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
void MGen::PopulateRequestHandlers(
    [[maybe_unused]] _Type* bluePrint,
    [[maybe_unused]] typename ::TasMdk::Request::Handler<_Type>::CollectionType& handlers)
{
    if (handlers.find("ope_reset") == handlers.end()) {
        handlers["ope_reset"] = [](_Type & component, [[maybe_unused]] ::Smp::IRequest* request) {
            /// Invoke ope_reset
            component.ope_reset();
        };
    }
}
} // namespace n

#endif // N_MGEN_H_
