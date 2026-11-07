// Block notice
// line two
// -----------------------------------------------------------------------------
// File Name    : AbstractOneGen.cpp
// -----------------------------------------------------------------------------
/// @file m/AbstractOneGen.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <m/AbstractOne.h>
#include <Smp/Publication/IPublishOperation.h>
#include <Smp/Version.h>

namespace m
{
::TasMdk::Request::Handler<AbstractOneGen>::CollectionType AbstractOneGen::requestHandlers;

void AbstractOneGen::Configure(::Smp::Services::ILogger* logger, ::Smp::Services::ILinkRegistry* linkRegistry) {
    // Call base implementation first
    ::TasMdk::Model::Configure(logger, linkRegistry);
    if (Configure_Callback) {
        Configure_Callback(logger, linkRegistry);
    }
}

void AbstractOneGen::Connect(::Smp::ISimulator* simulator) {
    // Call mdk implementation first
    ::TasMdk::Model::Connect(simulator);
    if (Connect_Callback) {
        Connect_Callback(simulator);
    }
}

void AbstractOneGen::Disconnect() {
    if (Disconnect_Callback) {
        Disconnect_Callback();
    }
    // Call parent implementation last, to remove references to the Simulator and its services
    ::TasMdk::Model::Disconnect();
}

AbstractOneGen::AbstractOneGen(
    ::Smp::String8 name,
    ::Smp::String8 description,
    ::Smp::IObject* parent,
    ::Smp::Publication::ITypeRegistry* type_registry)
    : // Base class initialization
    ::TasMdk::Model(name, description, parent, type_registry),
    // Field connectedState
    connectedState {false},
    // Field valueState
    valueState {}
{
}

AbstractOneGen::~AbstractOneGen() {
}

void AbstractOneGen::Publish(::Smp::IPublication* receiver) {
    // Call base class implementation first
    ::TasMdk::Model::Publish(receiver);

    if (Publish_Callback) {
        Publish_Callback(receiver);
    }

    // Populate the request handlers (only once)
    if (requestHandlers.empty()) {
        PopulateRequestHandlers<AbstractOneGen>(this, requestHandlers);
    }

    // Publish field connectedState
    receiver->PublishField(
        "connectedState", // Name
        "", // Description
        &connectedState, // Address
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        false // Output
    );

    // Publish field valueState
    receiver->PublishField(
        "valueState", // Name
        "", // Description
        &valueState, // Address
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        false // Output
    );

    // Publish operation mustImplement
    auto* op_mustImplement = receiver->PublishOperation(
        "mustImplement", // Name
        "", // Description
        ::Smp::ViewKind::VK_All // View Kind
    );
    op_mustImplement->PublishParameter(
        "return", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        Smp::Publication::ParameterDirectionKind::PDK_Return // Parameter Direction Kind
    );

    // Publish operation ping
    auto* op_ping = receiver->PublishOperation(
        "ping", // Name
        "", // Description
        ::Smp::ViewKind::VK_All // View Kind
    );
    op_ping->PublishParameter(
        "value", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        Smp::Publication::ParameterDirectionKind::PDK_In // Parameter Direction Kind
    );
    op_ping->PublishParameter(
        "return", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Bool, // Type UUID
        Smp::Publication::ParameterDirectionKind::PDK_Return // Parameter Direction Kind
    );

    // Publish Property connected
    receiver->PublishProperty(
        "connected", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Bool, // Type UUID
        ::Smp::AccessKind::AK_ReadWrite, // Access Kind
        ::Smp::ViewKind::VK_All // View Kind
    );

    // Publish Property ro
    receiver->PublishProperty(
        "ro", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        ::Smp::AccessKind::AK_ReadOnly, // Access Kind
        ::Smp::ViewKind::VK_All // View Kind
    );

    // Publish Property wo
    receiver->PublishProperty(
        "wo", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        ::Smp::AccessKind::AK_WriteOnly, // Access Kind
        ::Smp::ViewKind::VK_All // View Kind
    );
}

void AbstractOneGen::Invoke(::Smp::IRequest* request) {
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

const ::Smp::Uuid& AbstractOneGen::GetUuid() const {
    return Uuid_AbstractOne;
}

constexpr ::Smp::Int32 AbstractOneGen::K;

::Smp::Bool AbstractOneGen::get_connected(){
    return this->connectedState;
}
void AbstractOneGen::set_connected(::Smp::Bool value) {
    this->connectedState = value;
}

::Smp::Int32 AbstractOneGen::get_ro(){
    return this->valueState;
}

void AbstractOneGen::set_wo(::Smp::Int32 value) {
    this->valueState = value;
}
} // namespace m
