// -----------------------------------------------------------------------------
// File Name    : ProbeGen.cpp
// -----------------------------------------------------------------------------
/// @file defaults/ProbeGen.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <defaults/Probe.h>
#include <Smp/Version.h>

namespace defaults
{
::TasMdk::Request::Handler<ProbeGen>::CollectionType ProbeGen::requestHandlers;

void ProbeGen::Configure(::Smp::Services::ILogger* logger, ::Smp::Services::ILinkRegistry* linkRegistry) {
    // Call base implementation first
    ::TasMdk::Model::Configure(logger, linkRegistry);
    if (Configure_Callback) {
        Configure_Callback(logger, linkRegistry);
    }
}

void ProbeGen::Connect(::Smp::ISimulator* simulator) {
    // Call mdk implementation first
    ::TasMdk::Model::Connect(simulator);
    if (Connect_Callback) {
        Connect_Callback(simulator);
    }
}

void ProbeGen::Disconnect() {
    if (Disconnect_Callback) {
        Disconnect_Callback();
    }
    // Call parent implementation last, to remove references to the Simulator and its services
    ::TasMdk::Model::Disconnect();
}

ProbeGen::ProbeGen(
    ::Smp::String8 name,
    ::Smp::String8 description,
    ::Smp::IObject* parent,
    ::Smp::Publication::ITypeRegistry* type_registry)
    : // Base class initialization
    ::TasMdk::Model(name, description, parent, type_registry),
    // Field inp_scalar
    inp_scalar{
        "inp_scalar", // Name
        "", // Description
        this, // Parent
        ::Smp::ViewKind::VK_All, // View Kind
        type_registry, // Type Registry
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        7 // Default value
    },
    // Field out_scalar
    out_scalar{
        "out_scalar", // Name
        "", // Description
        this, // Parent
        ::Smp::ViewKind::VK_All, // View Kind
        type_registry, // Type Registry
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        7 // Default value
    },
    // Field inp_point
    inp_point{
        "inp_point", // Name
        "", // Description
        this, // Parent
        ::Smp::ViewKind::VK_All, // View Kind
        type_registry, // Type Registry
        ::defaults::Uuid_Point, // Type UUID
        { 1, 2} // Default value
    },
    // Field out_point
    out_point{
        "out_point", // Name
        "", // Description
        this, // Parent
        ::Smp::ViewKind::VK_All, // View Kind
        type_registry, // Type Registry
        ::defaults::Uuid_Point, // Type UUID
        { 1, 2} // Default value
    },
    // Field inp_packet
    inp_packet{
        "inp_packet", // Name
        "", // Description
        this, // Parent
        ::Smp::ViewKind::VK_All, // View Kind
        type_registry, // Type Registry
        ::defaults::Uuid_Packet, // Type UUID
        { {{ 1, 2}}, 3} // Default value
    },
    // Field out_packet
    out_packet{
        "out_packet", // Name
        "", // Description
        this, // Parent
        ::Smp::ViewKind::VK_All, // View Kind
        type_registry, // Type Registry
        ::defaults::Uuid_Packet, // Type UUID
        { {{ 1, 2}}, 3} // Default value
    },
    // Field inp_row
    inp_row{
        "inp_row", // Name
        "", // Description
        this, // Parent
        ::Smp::ViewKind::VK_All, // View Kind
        type_registry, // Type Registry
        ::defaults::Uuid_Row, // Type UUID
        { 1, 2} // Default value
    },
    // Field out_row
    out_row{
        "out_row", // Name
        "", // Description
        this, // Parent
        ::Smp::ViewKind::VK_All, // View Kind
        type_registry, // Type Registry
        ::defaults::Uuid_Row, // Type UUID
        { 1, 2} // Default value
    },
    // Field inp_matrix
    inp_matrix{
        "inp_matrix", // Name
        "", // Description
        this, // Parent
        ::Smp::ViewKind::VK_All, // View Kind
        type_registry, // Type Registry
        ::defaults::Uuid_Matrix, // Type UUID
        { {{ 1, 2}}, {{ 3, 4}}} // Default value
    },
    // Field out_matrix
    out_matrix{
        "out_matrix", // Name
        "", // Description
        this, // Parent
        ::Smp::ViewKind::VK_All, // View Kind
        type_registry, // Type Registry
        ::defaults::Uuid_Matrix, // Type UUID
        { {{ 1, 2}}, {{ 3, 4}}} // Default value
    },
    // Field inp_cube
    inp_cube{
        "inp_cube", // Name
        "", // Description
        this, // Parent
        ::Smp::ViewKind::VK_All, // View Kind
        type_registry, // Type Registry
        ::defaults::Uuid_Cube, // Type UUID
        { {{ {{ 1, 2}}, {{ 3, 4}}}}, {{ {{ 5, 6}}, {{ 7, 8}}}}} // Default value
    },
    // Field out_cube
    out_cube{
        "out_cube", // Name
        "", // Description
        this, // Parent
        ::Smp::ViewKind::VK_All, // View Kind
        type_registry, // Type Registry
        ::defaults::Uuid_Cube, // Type UUID
        { {{ {{ 1, 2}}, {{ 3, 4}}}}, {{ {{ 5, 6}}, {{ 7, 8}}}}} // Default value
    },
    // Field inp_hypercube
    inp_hypercube{
        "inp_hypercube", // Name
        "", // Description
        this, // Parent
        ::Smp::ViewKind::VK_All, // View Kind
        type_registry, // Type Registry
        ::defaults::Uuid_Hypercube, // Type UUID
        { {{ {{ {{ 1, 2}}, {{ 3, 4}}}}, {{ {{ 5, 6}}, {{ 7, 8}}}}}}, {{ {{ {{ 9, 10}}, {{ 11, 12}}}}, {{ {{ 13, 14}}, {{ 15, 16}}}}}}} // Default value
    },
    // Field out_hypercube
    out_hypercube{
        "out_hypercube", // Name
        "", // Description
        this, // Parent
        ::Smp::ViewKind::VK_All, // View Kind
        type_registry, // Type Registry
        ::defaults::Uuid_Hypercube, // Type UUID
        { {{ {{ {{ 1, 2}}, {{ 3, 4}}}}, {{ {{ 5, 6}}, {{ 7, 8}}}}}}, {{ {{ {{ 9, 10}}, {{ 11, 12}}}}, {{ {{ 13, 14}}, {{ 15, 16}}}}}}} // Default value
    },
    // Field inp_points
    inp_points{
        "inp_points", // Name
        "", // Description
        this, // Parent
        ::Smp::ViewKind::VK_All, // View Kind
        type_registry, // Type Registry
        ::defaults::Uuid_Points, // Type UUID
        { { 1, 2}, { 3, 4}} // Default value
    },
    // Field out_points
    out_points{
        "out_points", // Name
        "", // Description
        this, // Parent
        ::Smp::ViewKind::VK_All, // View Kind
        type_registry, // Type Registry
        ::defaults::Uuid_Points, // Type UUID
        { { 1, 2}, { 3, 4}} // Default value
    },
    // Field inp_packets
    inp_packets{
        "inp_packets", // Name
        "", // Description
        this, // Parent
        ::Smp::ViewKind::VK_All, // View Kind
        type_registry, // Type Registry
        ::defaults::Uuid_Packets, // Type UUID
        { { {{ 1, 2}}, 3}, { {{ 4, 5}}, 6}} // Default value
    },
    // Field out_packets
    out_packets{
        "out_packets", // Name
        "", // Description
        this, // Parent
        ::Smp::ViewKind::VK_All, // View Kind
        type_registry, // Type Registry
        ::defaults::Uuid_Packets, // Type UUID
        { { {{ 1, 2}}, 3}, { {{ 4, 5}}, 6}} // Default value
    },
    // Field inp_matrix_partial
    inp_matrix_partial{
        "inp_matrix_partial", // Name
        "", // Description
        this, // Parent
        ::Smp::ViewKind::VK_All, // View Kind
        type_registry, // Type Registry
        ::defaults::Uuid_Matrix, // Type UUID
        { {{ 1}}} // Default value
    },
    // Field out_matrix_partial
    out_matrix_partial{
        "out_matrix_partial", // Name
        "", // Description
        this, // Parent
        ::Smp::ViewKind::VK_All, // View Kind
        type_registry, // Type Registry
        ::defaults::Uuid_Matrix, // Type UUID
        { {{ 1}}} // Default value
    },
    // Field inp_points_partial
    inp_points_partial{
        "inp_points_partial", // Name
        "", // Description
        this, // Parent
        ::Smp::ViewKind::VK_All, // View Kind
        type_registry, // Type Registry
        ::defaults::Uuid_Points, // Type UUID
        { { 1}} // Default value
    },
    // Field out_points_partial
    out_points_partial{
        "out_points_partial", // Name
        "", // Description
        this, // Parent
        ::Smp::ViewKind::VK_All, // View Kind
        type_registry, // Type Registry
        ::defaults::Uuid_Points, // Type UUID
        { { 1}} // Default value
    },
    // Field inp_matrix_empty
    inp_matrix_empty{
        "inp_matrix_empty", // Name
        "", // Description
        this, // Parent
        ::Smp::ViewKind::VK_All, // View Kind
        type_registry, // Type Registry
        ::defaults::Uuid_Matrix, // Type UUID
        { } // Default value
    },
    // Field out_matrix_empty
    out_matrix_empty{
        "out_matrix_empty", // Name
        "", // Description
        this, // Parent
        ::Smp::ViewKind::VK_All, // View Kind
        type_registry, // Type Registry
        ::defaults::Uuid_Matrix, // Type UUID
        { } // Default value
    },
    // Field inp_matrix_absent
    inp_matrix_absent{
        "inp_matrix_absent", // Name
        "", // Description
        this, // Parent
        ::Smp::ViewKind::VK_All, // View Kind
        type_registry, // Type Registry
        ::defaults::Uuid_Matrix // Type UUID
    },
    // Field out_matrix_absent
    out_matrix_absent{
        "out_matrix_absent", // Name
        "", // Description
        this, // Parent
        ::Smp::ViewKind::VK_All, // View Kind
        type_registry, // Type Registry
        ::defaults::Uuid_Matrix // Type UUID
    },
    // Field sta_matrix
    sta_matrix {{ {{ 1, 2}}, {{ 3, 4}}}},
    // Field sta_points
    sta_points {{ { 1, 2}, { 3, 4}}}
{
}

ProbeGen::~ProbeGen() {
}

void ProbeGen::Publish(::Smp::IPublication* receiver) {
    // Call base class implementation first
    ::TasMdk::Model::Publish(receiver);

    if (Publish_Callback) {
        Publish_Callback(receiver);
    }

    // Populate the request handlers (only once)
    if (requestHandlers.empty()) {
        PopulateRequestHandlers<ProbeGen>(this, requestHandlers);
    }

    // Publish field inp_scalar
    receiver->PublishField(&inp_scalar);

    // Publish field out_scalar
    receiver->PublishField(&out_scalar);

    // Publish field inp_point
    receiver->PublishField(&inp_point);

    // Publish field out_point
    receiver->PublishField(&out_point);

    // Publish field inp_packet
    receiver->PublishField(&inp_packet);

    // Publish field out_packet
    receiver->PublishField(&out_packet);

    // Publish field inp_row
    receiver->PublishField(&inp_row);

    // Publish field out_row
    receiver->PublishField(&out_row);

    // Publish field inp_matrix
    receiver->PublishField(&inp_matrix);

    // Publish field out_matrix
    receiver->PublishField(&out_matrix);

    // Publish field inp_cube
    receiver->PublishField(&inp_cube);

    // Publish field out_cube
    receiver->PublishField(&out_cube);

    // Publish field inp_hypercube
    receiver->PublishField(&inp_hypercube);

    // Publish field out_hypercube
    receiver->PublishField(&out_hypercube);

    // Publish field inp_points
    receiver->PublishField(&inp_points);

    // Publish field out_points
    receiver->PublishField(&out_points);

    // Publish field inp_packets
    receiver->PublishField(&inp_packets);

    // Publish field out_packets
    receiver->PublishField(&out_packets);

    // Publish field inp_matrix_partial
    receiver->PublishField(&inp_matrix_partial);

    // Publish field out_matrix_partial
    receiver->PublishField(&out_matrix_partial);

    // Publish field inp_points_partial
    receiver->PublishField(&inp_points_partial);

    // Publish field out_points_partial
    receiver->PublishField(&out_points_partial);

    // Publish field inp_matrix_empty
    receiver->PublishField(&inp_matrix_empty);

    // Publish field out_matrix_empty
    receiver->PublishField(&out_matrix_empty);

    // Publish field inp_matrix_absent
    receiver->PublishField(&inp_matrix_absent);

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
}

void ProbeGen::Invoke(::Smp::IRequest* request) {
    if (request == nullptr) {
        return;
    }
    #if ECSS_SMP_VERSION < 202503L
    auto handler = requestHandlers.find(request->GetOperationName());
    #else
    auto handler = requestHandlers.find(request->GetName());
    #endif
    if (handler != requestHandlers.end()) {
        handler->second(*this, request);
    } else {
        // pass the request down to the base model
        ::TasMdk::Model::Invoke(request);
    }
}

const ::Smp::Uuid& ProbeGen::GetUuid() const {
    return Uuid_Probe;
}
} // namespace defaults
