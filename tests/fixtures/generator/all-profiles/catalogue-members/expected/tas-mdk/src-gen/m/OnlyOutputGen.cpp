// Block notice
// line two
// -----------------------------------------------------------------------------
// File Name    : OnlyOutputGen.cpp
// -----------------------------------------------------------------------------
/// @file m/OnlyOutputGen.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <m/OnlyOutput.h>
#include <Smp/Version.h>

namespace m
{
::TasMdk::Request::Handler<OnlyOutputGen>::CollectionType OnlyOutputGen::requestHandlers;

void OnlyOutputGen::Configure(::Smp::Services::ILogger* logger, ::Smp::Services::ILinkRegistry* linkRegistry) {
    // Call base implementation first
    ::TasMdk::Model::Configure(logger, linkRegistry);
    if (Configure_Callback) {
        Configure_Callback(logger, linkRegistry);
    }
}

void OnlyOutputGen::Connect(::Smp::ISimulator* simulator) {
    // Call mdk implementation first
    ::TasMdk::Model::Connect(simulator);
    if (Connect_Callback) {
        Connect_Callback(simulator);
    }
}

void OnlyOutputGen::Disconnect() {
    if (Disconnect_Callback) {
        Disconnect_Callback();
    }
    // Call parent implementation last, to remove references to the Simulator and its services
    ::TasMdk::Model::Disconnect();
}

OnlyOutputGen::OnlyOutputGen(
    ::Smp::String8 name,
    ::Smp::String8 description,
    ::Smp::IObject* parent,
    ::Smp::Publication::ITypeRegistry* type_registry)
    : // Base class initialization
    ::TasMdk::Model(name, description, parent, type_registry),
    // Field o
    o{
        "o", // Name
        "", // Description
        this, // Parent
        ::Smp::ViewKind::VK_All, // View Kind
        type_registry, // Type Registry
        ::Smp::Uuids::Uuid_Int32 // Type UUID
    },
    // Field f
    f{
        "f", // Name
        "", // Description
        this, // Parent
        ::Smp::ViewKind::VK_All, // View Kind
        type_registry, // Type Registry
        ::Smp::Uuids::Uuid_Bool // Type UUID
    }
{
}

OnlyOutputGen::~OnlyOutputGen() {
}

void OnlyOutputGen::Publish(::Smp::IPublication* receiver) {
    // Call base class implementation first
    ::TasMdk::Model::Publish(receiver);

    if (Publish_Callback) {
        Publish_Callback(receiver);
    }

    // Populate the request handlers (only once)
    if (requestHandlers.empty()) {
        PopulateRequestHandlers<OnlyOutputGen>(this, requestHandlers);
    }

    // Publish field o
    receiver->PublishField(&o);

    // Publish field f
    receiver->PublishField(&f);
}

void OnlyOutputGen::Invoke(::Smp::IRequest* request) {
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

const ::Smp::Uuid& OnlyOutputGen::GetUuid() const {
    return Uuid_OnlyOutput;
}
} // namespace m
