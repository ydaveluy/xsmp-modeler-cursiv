// -----------------------------------------------------------------------------
// File Name    : ChildGen.cpp
// -----------------------------------------------------------------------------
/// @file limits/ChildGen.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <limits/Child.h>
#include <Smp/Publication/IPublishOperation.h>
#include <Smp/Version.h>

namespace limits
{
::TasMdk::Request::Handler<ChildGen>::CollectionType ChildGen::requestHandlers;

void ChildGen::Configure(::Smp::Services::ILogger* logger, ::Smp::Services::ILinkRegistry* linkRegistry) {
    // Call base implementation first
    ::limits::Probe::Configure(logger, linkRegistry);
    if (Configure_Callback) {
        Configure_Callback(logger, linkRegistry);
    }
}

void ChildGen::Connect(::Smp::ISimulator* simulator) {
    // Call mdk implementation first
    ::limits::Probe::Connect(simulator);
    if (Connect_Callback) {
        Connect_Callback(simulator);
    }
}

void ChildGen::Disconnect() {
    if (Disconnect_Callback) {
        Disconnect_Callback();
    }
    // Call parent implementation last, to remove references to the Simulator and its services
    ::limits::Probe::Disconnect();
}

ChildGen::ChildGen(
    ::Smp::String8 name,
    ::Smp::String8 description,
    ::Smp::IObject* parent,
    ::Smp::Publication::ITypeRegistry* type_registry)
    : // Base class initialization
    ::limits::Probe(name, description, parent, type_registry)
{
}

ChildGen::~ChildGen() {
}

void ChildGen::Publish(::Smp::IPublication* receiver) {
    // Call base class implementation first
    ::limits::Probe::Publish(receiver);

    if (Publish_Callback) {
        Publish_Callback(receiver);
    }

    // Populate the request handlers (only once)
    if (requestHandlers.empty()) {
        PopulateRequestHandlers<ChildGen>(this, requestHandlers);
    }

    // WARNING: Operation normal is not invokable: overloaded operation names cannot be distinguished by Smp::IRequest.

    // Publish operation fresh
    auto* op_fresh = receiver->PublishOperation(
        "fresh", // Name
        "", // Description
        ::Smp::ViewKind::VK_All // View Kind
    );
    op_fresh->PublishParameter(
        "return", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        Smp::Publication::ParameterDirectionKind::PDK_Return // Parameter Direction Kind
    );
}

void ChildGen::Invoke(::Smp::IRequest* request) {
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
        ::limits::Probe::Invoke(request);
    }
}

const ::Smp::Uuid& ChildGen::GetUuid() const {
    return Uuid_Child;
}
} // namespace limits
