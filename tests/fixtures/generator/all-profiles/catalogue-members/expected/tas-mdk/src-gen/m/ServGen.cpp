// Block notice
// line two
// -----------------------------------------------------------------------------
// File Name    : ServGen.cpp
// -----------------------------------------------------------------------------
/// @file m/ServGen.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <m/Serv.h>
#include <Smp/Publication/IPublishOperation.h>
#include <Smp/Version.h>
#include <TasMdk/EntryPoint.h>
#include <TasMdk/EventSink.h>

namespace m
{
::TasMdk::Request::Handler<ServGen>::CollectionType ServGen::requestHandlers;

void ServGen::Configure(::Smp::Services::ILogger* logger, ::Smp::Services::ILinkRegistry* linkRegistry) {
    // Call base implementation first
    ::TasMdk::Service::Configure(logger, linkRegistry);
    if (Configure_Callback) {
        Configure_Callback(logger, linkRegistry);
    }
}

void ServGen::Connect(::Smp::ISimulator* simulator) {
    // Call mdk implementation first
    ::TasMdk::Service::Connect(simulator);
    if (Connect_Callback) {
        Connect_Callback(simulator);
    }
}

void ServGen::Disconnect() {
    if (Disconnect_Callback) {
        Disconnect_Callback();
    }
    // Call parent implementation last, to remove references to the Simulator and its services
    ::TasMdk::Service::Disconnect();
}

ServGen::ServGen(
    ::Smp::String8 name,
    ::Smp::String8 description,
    ::Smp::IObject* parent,
    ::Smp::Publication::ITypeRegistry* type_registry)
    : // Base class initialization
    ::TasMdk::Service(name, description, parent, type_registry),
    // EntryPoint: go
    go{ new ::TasMdk::EntryPoint(
        "go", // Name
        "", // Description
        this,
        _entrypoints,
        std::bind(&ServGen::_go, this))},
    // Event Sink: bang
    bang{ new ::TasMdk::EventSink(
        "bang", // Name
        "", // Description
        this,
        _event_sinks,
        std::bind(&ServGen::_bang, this, std::placeholders::_1))},
    // Event Source: boom
    boom{ new ::TasMdk::EventSource(
        "boom", // Name
        "", // Description
        this,
        _event_sources)},
    // Field connectedState
    connectedState {},
    // Container: kids
    kids {new ::TasMdk::Container<::Smp::IComposite>(
        "kids",
        "",
        this,
        _containers,
        1,
        1)},
    // Reference: peer
    peer {new ::TasMdk::Reference<::m::IBase>(
        "peer",
        "",
        this,
        _references,
        1,
        1)}
{
}

ServGen::~ServGen() {
    delete kids;
    kids = nullptr;
    delete peer;
    peer = nullptr;
    delete go;
    go = nullptr;
    delete bang;
    bang = nullptr;
    delete boom;
    boom = nullptr;
}

void ServGen::Publish(::Smp::IPublication* receiver) {
    // Call base class implementation first
    ::TasMdk::Service::Publish(receiver);

    if (Publish_Callback) {
        Publish_Callback(receiver);
    }

    // Populate the request handlers (only once)
    if (requestHandlers.empty()) {
        PopulateRequestHandlers<ServGen>(this, requestHandlers);
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

void ServGen::Invoke(::Smp::IRequest* request) {
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
        ::TasMdk::Service::Invoke(request);
    }
}

const ::Smp::Uuid& ServGen::GetUuid() const {
    return Uuid_Serv;
}

::Smp::Bool ServGen::get_connected(){
    return this->connectedState;
}
void ServGen::set_connected(::Smp::Bool value) {
    this->connectedState = value;
}
} // namespace m
