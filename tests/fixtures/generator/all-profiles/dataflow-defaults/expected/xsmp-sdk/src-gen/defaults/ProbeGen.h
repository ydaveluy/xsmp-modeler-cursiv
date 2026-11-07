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
#include <Smp/IComposite.h>
#include <Smp/ISimulator.h>
#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <Xsmp/Field.h>
#include <Xsmp/Model.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace defaults
{
/// Universally unique identifier of type Probe.
inline constexpr ::Smp::Uuid Uuid_Probe { 0x70000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000009U };

class ProbeGen: public ::Xsmp::Model
{
    friend class ::defaults::Probe;
    public:
    /// Constructor setting name, description, parent and simulator.
    /// @param name Name of new Model instance.
    /// @param description Description of new Model instance.
    /// @param parent Parent of new Model instance.
    /// @param simulator The simulator instance.
    ProbeGen(::Smp::String8 name,
            ::Smp::String8 description,
            ::Smp::IComposite* parent,
            ::Smp::ISimulator* simulator);
    /// deleted copy constructor
    ProbeGen(const ProbeGen&) = delete;
    /// deleted move constructor
    ProbeGen(ProbeGen&&) = delete;
    /// deleted copy assignment
    ProbeGen& operator=(const ProbeGen&) = delete;
    /// deleted move assignment
    ProbeGen& operator=(ProbeGen&&) = delete;

    /// Virtual destructor to release memory.
    ~ProbeGen() override = default;

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
    ::Smp::Int32 inp_scalar;
    ::Xsmp::Field<::Smp::Int32>::output out_scalar;
    ::defaults::Point inp_point;
    ::Xsmp::Field<::defaults::Point>::output out_point;
    ::defaults::Packet inp_packet;
    ::Xsmp::Field<::defaults::Packet>::output out_packet;
    ::defaults::Row inp_row;
    ::Xsmp::Field<::defaults::Row>::output out_row;
    ::defaults::Matrix inp_matrix;
    ::Xsmp::Field<::defaults::Matrix>::output out_matrix;
    ::defaults::Cube inp_cube;
    ::Xsmp::Field<::defaults::Cube>::output out_cube;
    ::defaults::Hypercube inp_hypercube;
    ::Xsmp::Field<::defaults::Hypercube>::output out_hypercube;
    ::defaults::Points inp_points;
    ::Xsmp::Field<::defaults::Points>::output out_points;
    ::defaults::Packets inp_packets;
    ::Xsmp::Field<::defaults::Packets>::output out_packets;
    ::defaults::Matrix inp_matrix_partial;
    ::Xsmp::Field<::defaults::Matrix>::output out_matrix_partial;
    ::defaults::Points inp_points_partial;
    ::Xsmp::Field<::defaults::Points>::output out_points_partial;
    ::defaults::Matrix inp_matrix_empty;
    ::Xsmp::Field<::defaults::Matrix>::output out_matrix_empty;
    ::defaults::Matrix inp_matrix_absent;
    ::Xsmp::Field<::defaults::Matrix>::output out_matrix_absent;
    ::defaults::Matrix sta_matrix;
    ::defaults::Points sta_points;
};
} // namespace defaults

#endif // DEFAULTS_PROBEGEN_H_
