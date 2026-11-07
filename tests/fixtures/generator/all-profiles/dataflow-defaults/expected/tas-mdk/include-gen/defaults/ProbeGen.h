// -----------------------------------------------------------------------------
// File Name    : ProbeGen.h
// -----------------------------------------------------------------------------
/// @file defaults/ProbeGen.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef DEFAULTS_PROBEGEN_H_
#define DEFAULTS_PROBEGEN_H_

// ----------------------------------------------------------------------------
// --------------------------- Forward Declarations ---------------------------
// ----------------------------------------------------------------------------

namespace defaults
{
class Probe;
} // namespace defaults

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <defaults/Cube.h>
#include <defaults/Hypercube.h>
#include <defaults/Matrix.h>
#include <defaults/Packet.h>
#include <defaults/Packets.h>
#include <defaults/Point.h>
#include <defaults/Points.h>
#include <defaults/Row.h>
#include <functional>
#include <Smp/ISimulator.h>
#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <TasMdk/Field.h>
#include <type_traits>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace defaults
{
/// Universally unique identifier of type Probe.
constexpr ::Smp::Uuid Uuid_Probe { 0x70000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000009U };

class ProbeGen: public ::TasMdk::Model
{
    friend class ::defaults::Probe;

    typename ::TasMdk::SimpleField<::Smp::Int32>::input::type inp_scalar;
    typename ::TasMdk::SimpleField<::Smp::Int32>::output::type out_scalar;
    typename ::TasMdk::StructureField<::defaults::_Point>::input::type inp_point;
    typename ::TasMdk::StructureField<::defaults::_Point>::output::type out_point;
    typename ::TasMdk::StructureField<::defaults::_Packet>::input::type inp_packet;
    typename ::TasMdk::StructureField<::defaults::_Packet>::output::type out_packet;
    typename ::TasMdk::ArrayField<::defaults::_Row>::input::type inp_row;
    typename ::TasMdk::ArrayField<::defaults::_Row>::output::type out_row;
    typename ::TasMdk::ArrayField<::defaults::_Matrix>::input::type inp_matrix;
    typename ::TasMdk::ArrayField<::defaults::_Matrix>::output::type out_matrix;
    typename ::TasMdk::ArrayField<::defaults::_Cube>::input::type inp_cube;
    typename ::TasMdk::ArrayField<::defaults::_Cube>::output::type out_cube;
    typename ::TasMdk::ArrayField<::defaults::_Hypercube>::input::type inp_hypercube;
    typename ::TasMdk::ArrayField<::defaults::_Hypercube>::output::type out_hypercube;
    typename ::TasMdk::ArrayField<::defaults::_Points>::input::type inp_points;
    typename ::TasMdk::ArrayField<::defaults::_Points>::output::type out_points;
    typename ::TasMdk::ArrayField<::defaults::_Packets>::input::type inp_packets;
    typename ::TasMdk::ArrayField<::defaults::_Packets>::output::type out_packets;
    typename ::TasMdk::ArrayField<::defaults::_Matrix>::input::type inp_matrix_partial;
    typename ::TasMdk::ArrayField<::defaults::_Matrix>::output::type out_matrix_partial;
    typename ::TasMdk::ArrayField<::defaults::_Points>::input::type inp_points_partial;
    typename ::TasMdk::ArrayField<::defaults::_Points>::output::type out_points_partial;
    typename ::TasMdk::ArrayField<::defaults::_Matrix>::input::type inp_matrix_empty;
    typename ::TasMdk::ArrayField<::defaults::_Matrix>::output::type out_matrix_empty;
    typename ::TasMdk::ArrayField<::defaults::_Matrix>::input::type inp_matrix_absent;
    typename ::TasMdk::ArrayField<::defaults::_Matrix>::output::type out_matrix_absent;
    ::defaults::Matrix sta_matrix;
    ::defaults::Points sta_points;

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
}
} // namespace defaults

#endif // DEFAULTS_PROBEGEN_H_
