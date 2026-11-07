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
#include <Xsmp/ComponentHelper.h>

namespace defaults
{
//--------------------------- Constructor -------------------------
ProbeGen::ProbeGen(
    ::Smp::String8 name,
    ::Smp::String8 description,
    ::Smp::IComposite* parent,
    ::Smp::ISimulator* simulator):
    // Base class
    ::Xsmp::Model(name, description, parent, simulator),
    // Field inp_scalar
    inp_scalar {7},
    // Field out_scalar
    out_scalar{
        simulator->GetTypeRegistry(), // Type Registry
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        "out_scalar", // Name
        "", // Description
        this, // Parent
        ::Smp::ViewKind::VK_All, // View Kind
        7 // Default value
    },
    // Field inp_point
    inp_point { 1, 2},
    // Field out_point
    out_point{
        simulator->GetTypeRegistry(), // Type Registry
        ::defaults::Uuid_Point, // Type UUID
        "out_point", // Name
        "", // Description
        this, // Parent
        ::Smp::ViewKind::VK_All, // View Kind
        { 1, 2} // Default value
    },
    // Field inp_packet
    inp_packet { {{ 1, 2}}, 3},
    // Field out_packet
    out_packet{
        simulator->GetTypeRegistry(), // Type Registry
        ::defaults::Uuid_Packet, // Type UUID
        "out_packet", // Name
        "", // Description
        this, // Parent
        ::Smp::ViewKind::VK_All, // View Kind
        { {{ 1, 2}}, 3} // Default value
    },
    // Field inp_row
    inp_row {{ 1, 2}},
    // Field out_row
    out_row{
        simulator->GetTypeRegistry(), // Type Registry
        ::defaults::Uuid_Row, // Type UUID
        "out_row", // Name
        "", // Description
        this, // Parent
        ::Smp::ViewKind::VK_All, // View Kind
        {{ 1, 2}} // Default value
    },
    // Field inp_matrix
    inp_matrix {{ {{ 1, 2}}, {{ 3, 4}}}},
    // Field out_matrix
    out_matrix{
        simulator->GetTypeRegistry(), // Type Registry
        ::defaults::Uuid_Matrix, // Type UUID
        "out_matrix", // Name
        "", // Description
        this, // Parent
        ::Smp::ViewKind::VK_All, // View Kind
        {{ {{ 1, 2}}, {{ 3, 4}}}} // Default value
    },
    // Field inp_cube
    inp_cube {{ {{ {{ 1, 2}}, {{ 3, 4}}}}, {{ {{ 5, 6}}, {{ 7, 8}}}}}},
    // Field out_cube
    out_cube{
        simulator->GetTypeRegistry(), // Type Registry
        ::defaults::Uuid_Cube, // Type UUID
        "out_cube", // Name
        "", // Description
        this, // Parent
        ::Smp::ViewKind::VK_All, // View Kind
        {{ {{ {{ 1, 2}}, {{ 3, 4}}}}, {{ {{ 5, 6}}, {{ 7, 8}}}}}} // Default value
    },
    // Field inp_hypercube
    inp_hypercube {{ {{ {{ {{ 1, 2}}, {{ 3, 4}}}}, {{ {{ 5, 6}}, {{ 7, 8}}}}}}, {{ {{ {{ 9, 10}}, {{ 11, 12}}}}, {{ {{ 13, 14}}, {{ 15, 16}}}}}}}},
    // Field out_hypercube
    out_hypercube{
        simulator->GetTypeRegistry(), // Type Registry
        ::defaults::Uuid_Hypercube, // Type UUID
        "out_hypercube", // Name
        "", // Description
        this, // Parent
        ::Smp::ViewKind::VK_All, // View Kind
        {{ {{ {{ {{ 1, 2}}, {{ 3, 4}}}}, {{ {{ 5, 6}}, {{ 7, 8}}}}}}, {{ {{ {{ 9, 10}}, {{ 11, 12}}}}, {{ {{ 13, 14}}, {{ 15, 16}}}}}}}} // Default value
    },
    // Field inp_points
    inp_points {{ { 1, 2}, { 3, 4}}},
    // Field out_points
    out_points{
        simulator->GetTypeRegistry(), // Type Registry
        ::defaults::Uuid_Points, // Type UUID
        "out_points", // Name
        "", // Description
        this, // Parent
        ::Smp::ViewKind::VK_All, // View Kind
        {{ { 1, 2}, { 3, 4}}} // Default value
    },
    // Field inp_packets
    inp_packets {{ { {{ 1, 2}}, 3}, { {{ 4, 5}}, 6}}},
    // Field out_packets
    out_packets{
        simulator->GetTypeRegistry(), // Type Registry
        ::defaults::Uuid_Packets, // Type UUID
        "out_packets", // Name
        "", // Description
        this, // Parent
        ::Smp::ViewKind::VK_All, // View Kind
        {{ { {{ 1, 2}}, 3}, { {{ 4, 5}}, 6}}} // Default value
    },
    // Field inp_matrix_partial
    inp_matrix_partial {{ {{ 1}}}},
    // Field out_matrix_partial
    out_matrix_partial{
        simulator->GetTypeRegistry(), // Type Registry
        ::defaults::Uuid_Matrix, // Type UUID
        "out_matrix_partial", // Name
        "", // Description
        this, // Parent
        ::Smp::ViewKind::VK_All, // View Kind
        {{ {{ 1}}}} // Default value
    },
    // Field inp_points_partial
    inp_points_partial {{ { 1}}},
    // Field out_points_partial
    out_points_partial{
        simulator->GetTypeRegistry(), // Type Registry
        ::defaults::Uuid_Points, // Type UUID
        "out_points_partial", // Name
        "", // Description
        this, // Parent
        ::Smp::ViewKind::VK_All, // View Kind
        {{ { 1}}} // Default value
    },
    // Field inp_matrix_empty
    inp_matrix_empty {{ }},
    // Field out_matrix_empty
    out_matrix_empty{
        simulator->GetTypeRegistry(), // Type Registry
        ::defaults::Uuid_Matrix, // Type UUID
        "out_matrix_empty", // Name
        "", // Description
        this, // Parent
        ::Smp::ViewKind::VK_All, // View Kind
        {{ }} // Default value
    },
    // Field inp_matrix_absent
    inp_matrix_absent {},
    // Field out_matrix_absent
    out_matrix_absent{
        simulator->GetTypeRegistry(), // Type Registry
        ::defaults::Uuid_Matrix, // Type UUID
        "out_matrix_absent", // Name
        "", // Description
        this, // Parent
        ::Smp::ViewKind::VK_All // View Kind
    },
    // Field sta_matrix
    sta_matrix {{ {{ 1, 2}}, {{ 3, 4}}}},
    // Field sta_points
    sta_points {{ { 1, 2}, { 3, 4}}} { }

void ProbeGen::Publish(::Smp::IPublication* receiver) {
    // Call parent class implementation first
    ::Xsmp::Model::Publish(receiver);

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
    receiver->PublishField(&out_scalar);

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
    receiver->PublishField(&out_point);

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
    receiver->PublishField(&out_packet);

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
    receiver->PublishField(&out_row);

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
    receiver->PublishField(&out_matrix);

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
    receiver->PublishField(&out_cube);

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
    receiver->PublishField(&out_hypercube);

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
    receiver->PublishField(&out_points);

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
    receiver->PublishField(&out_packets);

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
    receiver->PublishField(&out_matrix_partial);

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
    receiver->PublishField(&out_points_partial);

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
    receiver->PublishField(&out_matrix_empty);

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
    receiver->PublishField(&out_matrix_absent);

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

    // Call user DoPublish if any
    ::Xsmp::Component::Helper::Publish<::defaults::Probe>(this, receiver);
}

void ProbeGen::Configure(::Smp::Services::ILogger* logger, ::Smp::Services::ILinkRegistry* linkRegistry) {
    // Call parent implementation first
    ::Xsmp::Model::Configure(logger, linkRegistry);

    // Call user DoConfigure if any
    ::Xsmp::Component::Helper::Configure<::defaults::Probe>(this, logger, linkRegistry);
}

void ProbeGen::Connect(::Smp::ISimulator* simulator) {
    // Call parent implementation first
    ::Xsmp::Model::Connect(simulator);

    // Call user DoConnect if any
    ::Xsmp::Component::Helper::Connect<::defaults::Probe>(this, simulator);
}

void ProbeGen::Disconnect() {
    if (this->::Xsmp::Component::GetState() == ::Smp::ComponentStateKind::CSK_Connected) {
        // Call user DoDisconnect if any
        ::Xsmp::Component::Helper::Disconnect<::defaults::Probe>(this);
    }

    // Call parent implementation last, to remove references to the Simulator and its services
    ::Xsmp::Model::Disconnect();
}

const ::Smp::Uuid& ProbeGen::GetUuid() const {
    return Uuid_Probe;
}
} // namespace defaults
