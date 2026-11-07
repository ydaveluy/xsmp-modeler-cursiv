// Block notice
// line two
// -----------------------------------------------------------------------------
// File Name    : ServKidGen.h
// -----------------------------------------------------------------------------
/// @file m/ServKidGen.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef M_SERVKIDGEN_H_
#define M_SERVKIDGEN_H_

// ----------------------------------------------------------------------------
// --------------------------- Forward Declarations ---------------------------
// ----------------------------------------------------------------------------

namespace m
{
class ServKid;
} // namespace m

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <functional>
#include <m/Serv.h>
#include <Smp/ISimulator.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <type_traits>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace m
{
/// Universally unique identifier of type ServKid.
constexpr ::Smp::Uuid Uuid_ServKid { 0x20000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000023U };

class ServKidGen: public ::m::Serv
{
    friend class ::m::ServKid;

    // ------------------------------------------------------------------------------------
    // ------------------------- Constructors/Destructor -------------------------
    // ------------------------------------------------------------------------------------
    public:
    /// Constructor setting name, description and parent.
    /// @param name Name of new model instance.
    /// @param description Description of new model instance.
    /// @param parent Parent of new model instance.
    /// @param type_registry Reference to global type registry.
    ServKidGen(
            ::Smp::String8 name,
            ::Smp::String8 description,
            ::Smp::IObject* parent,
            ::Smp::Publication::ITypeRegistry* type_registry);

    /// Virtual destructor to release memory.
    ~ServKidGen() override;

    static ::TasMdk::Request::Handler<ServKidGen>::CollectionType requestHandlers;

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
void ServKidGen::PopulateRequestHandlers(
    [[maybe_unused]] _Type* bluePrint,
    [[maybe_unused]] typename ::TasMdk::Request::Handler<_Type>::CollectionType& handlers)
{
}
} // namespace m

#endif // M_SERVKIDGEN_H_
