// -----------------------------------------------------------------------------
// File Name    : ProbeGen.cpp
// -----------------------------------------------------------------------------
/// @file defaults/ProbeGen.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <defaults/Probe.h>
#include <Smp/ViewKind.h>

namespace defaults
{
//--------------------------- Constructor -------------------------
ProbeGen::ProbeGen(
    ::Smp::String8 name,
    ::Smp::String8 description,
    ::Smp::IComposite* parent,
    ::Smp::ISimulator* simulator):
    // Base class
    ::esa::ecss::smp::cdk::Model(name, description, parent, simulator),
    // Field inp_scalar
    inp_scalar {7},
    // Field out_scalar
    out_scalar {7},
    // Field inp_point
    inp_point { 1, 2},
    // Field out_point
    out_point { 1, 2},
    // Field inp_packet
    inp_packet { {{ 1, 2}}, 3},
    // Field out_packet
    out_packet { {{ 1, 2}}, 3},
    // Field inp_row
    inp_row {{ 1, 2}},
    // Field out_row
    out_row {{ 1, 2}},
    // Field inp_matrix
    inp_matrix {{ {{ 1, 2}}, {{ 3, 4}}}},
    // Field out_matrix
    out_matrix {{ {{ 1, 2}}, {{ 3, 4}}}},
    // Field inp_cube
    inp_cube {{ {{ {{ 1, 2}}, {{ 3, 4}}}}, {{ {{ 5, 6}}, {{ 7, 8}}}}}},
    // Field out_cube
    out_cube {{ {{ {{ 1, 2}}, {{ 3, 4}}}}, {{ {{ 5, 6}}, {{ 7, 8}}}}}},
    // Field inp_hypercube
    inp_hypercube {{ {{ {{ {{ 1, 2}}, {{ 3, 4}}}}, {{ {{ 5, 6}}, {{ 7, 8}}}}}}, {{ {{ {{ 9, 10}}, {{ 11, 12}}}}, {{ {{ 13, 14}}, {{ 15, 16}}}}}}}},
    // Field out_hypercube
    out_hypercube {{ {{ {{ {{ 1, 2}}, {{ 3, 4}}}}, {{ {{ 5, 6}}, {{ 7, 8}}}}}}, {{ {{ {{ 9, 10}}, {{ 11, 12}}}}, {{ {{ 13, 14}}, {{ 15, 16}}}}}}}},
    // Field inp_points
    inp_points {{ { 1, 2}, { 3, 4}}},
    // Field out_points
    out_points {{ { 1, 2}, { 3, 4}}},
    // Field inp_packets
    inp_packets {{ { {{ 1, 2}}, 3}, { {{ 4, 5}}, 6}}},
    // Field out_packets
    out_packets {{ { {{ 1, 2}}, 3}, { {{ 4, 5}}, 6}}},
    // Field inp_matrix_partial
    inp_matrix_partial {{ {{ 1}}}},
    // Field out_matrix_partial
    out_matrix_partial {{ {{ 1}}}},
    // Field inp_points_partial
    inp_points_partial {{ { 1}}},
    // Field out_points_partial
    out_points_partial {{ { 1}}},
    // Field inp_matrix_empty
    inp_matrix_empty {{ }},
    // Field out_matrix_empty
    out_matrix_empty {{ }},
    // Field inp_matrix_absent
    inp_matrix_absent {},
    // Field out_matrix_absent
    out_matrix_absent {},
    // Field sta_matrix
    sta_matrix {{ {{ 1, 2}}, {{ 3, 4}}}},
    // Field sta_points
    sta_points {{ { 1, 2}, { 3, 4}}} {
}

/// Virtual destructor that is called by inherited classes as well.
ProbeGen::~ProbeGen() {
}

void ProbeGen::Publish(::Smp::IPublication* receiver) {
    // Call parent class implementation first
    ::esa::ecss::smp::cdk::Model::Publish(receiver);

    // Publish field inp_scalar
    receiver->PublishField(
        "inp_scalar", // Name
        "", // Description
        &inp_scalar, // Address
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        true, // Input
        false // Output
    );

    // Publish field out_scalar
    receiver->PublishField(
        "out_scalar", // Name
        "", // Description
        &out_scalar, // Address
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        true // Output
    );

    // Publish field inp_point
    receiver->PublishField(
        "inp_point", // Name
        "", // Description
        &inp_point, // Address
        ::defaults::Uuid_Point, // Type UUID
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        true, // Input
        false // Output
    );

    // Publish field out_point
    receiver->PublishField(
        "out_point", // Name
        "", // Description
        &out_point, // Address
        ::defaults::Uuid_Point, // Type UUID
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        true // Output
    );

    // Publish field inp_packet
    receiver->PublishField(
        "inp_packet", // Name
        "", // Description
        &inp_packet, // Address
        ::defaults::Uuid_Packet, // Type UUID
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        true, // Input
        false // Output
    );

    // Publish field out_packet
    receiver->PublishField(
        "out_packet", // Name
        "", // Description
        &out_packet, // Address
        ::defaults::Uuid_Packet, // Type UUID
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        true // Output
    );

    // Publish field inp_row
    receiver->PublishField(
        "inp_row", // Name
        "", // Description
        &inp_row, // Address
        ::defaults::Uuid_Row, // Type UUID
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        true, // Input
        false // Output
    );

    // Publish field out_row
    receiver->PublishField(
        "out_row", // Name
        "", // Description
        &out_row, // Address
        ::defaults::Uuid_Row, // Type UUID
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        true // Output
    );

    // Publish field inp_matrix
    receiver->PublishField(
        "inp_matrix", // Name
        "", // Description
        &inp_matrix, // Address
        ::defaults::Uuid_Matrix, // Type UUID
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        true, // Input
        false // Output
    );

    // Publish field out_matrix
    receiver->PublishField(
        "out_matrix", // Name
        "", // Description
        &out_matrix, // Address
        ::defaults::Uuid_Matrix, // Type UUID
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        true // Output
    );

    // Publish field inp_cube
    receiver->PublishField(
        "inp_cube", // Name
        "", // Description
        &inp_cube, // Address
        ::defaults::Uuid_Cube, // Type UUID
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        true, // Input
        false // Output
    );

    // Publish field out_cube
    receiver->PublishField(
        "out_cube", // Name
        "", // Description
        &out_cube, // Address
        ::defaults::Uuid_Cube, // Type UUID
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        true // Output
    );

    // Publish field inp_hypercube
    receiver->PublishField(
        "inp_hypercube", // Name
        "", // Description
        &inp_hypercube, // Address
        ::defaults::Uuid_Hypercube, // Type UUID
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        true, // Input
        false // Output
    );

    // Publish field out_hypercube
    receiver->PublishField(
        "out_hypercube", // Name
        "", // Description
        &out_hypercube, // Address
        ::defaults::Uuid_Hypercube, // Type UUID
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        true // Output
    );

    // Publish field inp_points
    receiver->PublishField(
        "inp_points", // Name
        "", // Description
        &inp_points, // Address
        ::defaults::Uuid_Points, // Type UUID
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        true, // Input
        false // Output
    );

    // Publish field out_points
    receiver->PublishField(
        "out_points", // Name
        "", // Description
        &out_points, // Address
        ::defaults::Uuid_Points, // Type UUID
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        true // Output
    );

    // Publish field inp_packets
    receiver->PublishField(
        "inp_packets", // Name
        "", // Description
        &inp_packets, // Address
        ::defaults::Uuid_Packets, // Type UUID
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        true, // Input
        false // Output
    );

    // Publish field out_packets
    receiver->PublishField(
        "out_packets", // Name
        "", // Description
        &out_packets, // Address
        ::defaults::Uuid_Packets, // Type UUID
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        true // Output
    );

    // Publish field inp_matrix_partial
    receiver->PublishField(
        "inp_matrix_partial", // Name
        "", // Description
        &inp_matrix_partial, // Address
        ::defaults::Uuid_Matrix, // Type UUID
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        true, // Input
        false // Output
    );

    // Publish field out_matrix_partial
    receiver->PublishField(
        "out_matrix_partial", // Name
        "", // Description
        &out_matrix_partial, // Address
        ::defaults::Uuid_Matrix, // Type UUID
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        true // Output
    );

    // Publish field inp_points_partial
    receiver->PublishField(
        "inp_points_partial", // Name
        "", // Description
        &inp_points_partial, // Address
        ::defaults::Uuid_Points, // Type UUID
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        true, // Input
        false // Output
    );

    // Publish field out_points_partial
    receiver->PublishField(
        "out_points_partial", // Name
        "", // Description
        &out_points_partial, // Address
        ::defaults::Uuid_Points, // Type UUID
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        true // Output
    );

    // Publish field inp_matrix_empty
    receiver->PublishField(
        "inp_matrix_empty", // Name
        "", // Description
        &inp_matrix_empty, // Address
        ::defaults::Uuid_Matrix, // Type UUID
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        true, // Input
        false // Output
    );

    // Publish field out_matrix_empty
    receiver->PublishField(
        "out_matrix_empty", // Name
        "", // Description
        &out_matrix_empty, // Address
        ::defaults::Uuid_Matrix, // Type UUID
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        true // Output
    );

    // Publish field inp_matrix_absent
    receiver->PublishField(
        "inp_matrix_absent", // Name
        "", // Description
        &inp_matrix_absent, // Address
        ::defaults::Uuid_Matrix, // Type UUID
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        true, // Input
        false // Output
    );

    // Publish field out_matrix_absent
    receiver->PublishField(
        "out_matrix_absent", // Name
        "", // Description
        &out_matrix_absent, // Address
        ::defaults::Uuid_Matrix, // Type UUID
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        true // Output
    );

    // Publish field sta_matrix
    receiver->PublishField(
        "sta_matrix", // Name
        "", // Description
        &sta_matrix, // Address
        ::defaults::Uuid_Matrix, // Type UUID
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        false // Output
    );

    // Publish field sta_points
    receiver->PublishField(
        "sta_points", // Name
        "", // Description
        &sta_points, // Address
        ::defaults::Uuid_Points, // Type UUID
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        false // Output
    );

    dynamic_cast<::defaults::Probe*>(this)->DoPublish(receiver);
}

void ProbeGen::Configure(::Smp::Services::ILogger* logger, ::Smp::Services::ILinkRegistry* linkRegistry) {
    // Call parent implementation first
    ::esa::ecss::smp::cdk::Model::Configure(logger, linkRegistry);

    dynamic_cast<::defaults::Probe*>(this)->DoConfigure(logger, linkRegistry);
}

void ProbeGen::Connect(::Smp::ISimulator* simulator) {
    // Call parent implementation first
    ::esa::ecss::smp::cdk::Model::Connect(simulator);

    dynamic_cast<::defaults::Probe*>(this)->DoConnect(simulator);
}

void ProbeGen::Disconnect() {
    if (this->GetState() == ::Smp::ComponentStateKind::CSK_Connected) {
        dynamic_cast<::defaults::Probe*>(this)->DoDisconnect();
    }

    // Call parent implementation last, to remove references to the Simulator and its services
    ::esa::ecss::smp::cdk::Model::Disconnect();
}

const ::Smp::Uuid& ProbeGen::GetUuid() const {
    return Uuid_Probe;
}
} // namespace defaults
