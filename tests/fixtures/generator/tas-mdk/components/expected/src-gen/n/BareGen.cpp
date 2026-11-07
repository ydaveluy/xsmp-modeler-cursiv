// -----------------------------------------------------------------------------
// File Name    : BareGen.cpp
// -----------------------------------------------------------------------------
/// @file n/BareGen.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <n/Bare.h>
#include <Smp/Publication/IPublishOperation.h>
#include <Smp/Version.h>

namespace n
{
::TasMdk::Request::Handler<BareGen>::CollectionType BareGen::requestHandlers;

void BareGen::Configure(::Smp::Services::ILogger* logger, ::Smp::Services::ILinkRegistry* linkRegistry) {
    // Call base implementation first
    ::TasMdk::Model::Configure(logger, linkRegistry);
    if (Configure_Callback) {
        Configure_Callback(logger, linkRegistry);
    }
}

void BareGen::Connect(::Smp::ISimulator* simulator) {
    // Call mdk implementation first
    ::TasMdk::Model::Connect(simulator);
    if (Connect_Callback) {
        Connect_Callback(simulator);
    }
}

void BareGen::Disconnect() {
    if (Disconnect_Callback) {
        Disconnect_Callback();
    }
    // Call parent implementation last, to remove references to the Simulator and its services
    ::TasMdk::Model::Disconnect();
}

BareGen::BareGen(
    ::Smp::String8 name,
    ::Smp::String8 description,
    ::Smp::IObject* parent,
    ::Smp::Publication::ITypeRegistry* type_registry)
    : // Base class initialization
    ::TasMdk::Model(name, description, parent, type_registry)
{
}

BareGen::~BareGen() {
}

void BareGen::Publish(::Smp::IPublication* receiver) {
    // Call base class implementation first
    ::TasMdk::Model::Publish(receiver);

    if (Publish_Callback) {
        Publish_Callback(receiver);
    }

    // Populate the request handlers (only once)
    if (requestHandlers.empty()) {
        PopulateRequestHandlers<BareGen>(this, requestHandlers);
    }

    // Publish operation reset
    receiver->PublishOperation(
        "reset", // Name
        "", // Description
        ::Smp::ViewKind::VK_All // View Kind
    );
}

void BareGen::Invoke(::Smp::IRequest* request) {
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

const ::Smp::Uuid& BareGen::GetUuid() const {
    return Uuid_Bare;
}
} // namespace n
