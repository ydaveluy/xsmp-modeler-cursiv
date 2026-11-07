// -----------------------------------------------------------------------------
// File Name    : ProbeGen.cpp
// -----------------------------------------------------------------------------
/// @file optional/ProbeGen.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <optional/Probe.h>
#include <Smp/Publication/IPublishOperation.h>
#include <Smp/Version.h>

namespace optional
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
    ::TasMdk::Model(name, description, parent, type_registry)
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

    // Publish operation ope_input
    auto* op_ope_input = receiver->PublishOperation(
        "ope_input", // Name
        "Optional input.", // Description
        ::Smp::ViewKind::VK_All // View Kind
    );
    op_ope_input->PublishParameter(
        "value", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        Smp::Publication::ParameterDirectionKind::PDK_In // Parameter Direction Kind
    );

    // Publish operation ope_string
    auto* op_ope_string = receiver->PublishOperation(
        "ope_string", // Name
        "The request owns the String8 until the handler returns.", // Description
        ::Smp::ViewKind::VK_All // View Kind
    );
    op_ope_string->PublishParameter(
        "value", // Name
        "", // Description
        ::Smp::Uuids::Uuid_String8, // Type UUID
        Smp::Publication::ParameterDirectionKind::PDK_In // Parameter Direction Kind
    );

    // Publish operation ope_output
    auto* op_ope_output = receiver->PublishOperation(
        "ope_output", // Name
        "Optional output.", // Description
        ::Smp::ViewKind::VK_All // View Kind
    );
    op_ope_output->PublishParameter(
        "value", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        Smp::Publication::ParameterDirectionKind::PDK_Out // Parameter Direction Kind
    );

    // Publish operation ope_inout
    auto* op_ope_inout = receiver->PublishOperation(
        "ope_inout", // Name
        "Optional input/output.", // Description
        ::Smp::ViewKind::VK_All // View Kind
    );
    op_ope_inout->PublishParameter(
        "value", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        Smp::Publication::ParameterDirectionKind::PDK_InOut // Parameter Direction Kind
    );

    // Publish operation ope_names
    auto* op_ope_names = receiver->PublishOperation(
        "ope_names", // Name
        "The parameter names must not collide with generated presence flags.", // Description
        ::Smp::ViewKind::VK_All // View Kind
    );
    op_ope_names->PublishParameter(
        "value", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        Smp::Publication::ParameterDirectionKind::PDK_In // Parameter Direction Kind
    );
    op_ope_names->PublishParameter(
        "value_present", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        Smp::Publication::ParameterDirectionKind::PDK_In // Parameter Direction Kind
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
} // namespace optional
