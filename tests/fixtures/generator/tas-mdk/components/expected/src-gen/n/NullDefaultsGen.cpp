// -----------------------------------------------------------------------------
// File Name    : NullDefaultsGen.cpp
// -----------------------------------------------------------------------------
/// @file n/NullDefaultsGen.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <n/NullDefaults.h>
#include <Smp/Publication/IPublishOperation.h>
#include <Smp/Version.h>

namespace n
{
::TasMdk::Request::Handler<NullDefaultsGen>::CollectionType NullDefaultsGen::requestHandlers;

void NullDefaultsGen::Configure(::Smp::Services::ILogger* logger, ::Smp::Services::ILinkRegistry* linkRegistry) {
    // Call base implementation first
    ::TasMdk::Model::Configure(logger, linkRegistry);
    if (Configure_Callback) {
        Configure_Callback(logger, linkRegistry);
    }
}

void NullDefaultsGen::Connect(::Smp::ISimulator* simulator) {
    // Call mdk implementation first
    ::TasMdk::Model::Connect(simulator);
    if (Connect_Callback) {
        Connect_Callback(simulator);
    }
}

void NullDefaultsGen::Disconnect() {
    if (Disconnect_Callback) {
        Disconnect_Callback();
    }
    // Call parent implementation last, to remove references to the Simulator and its services
    ::TasMdk::Model::Disconnect();
}

NullDefaultsGen::NullDefaultsGen(
    ::Smp::String8 name,
    ::Smp::String8 description,
    ::Smp::IObject* parent,
    ::Smp::Publication::ITypeRegistry* type_registry)
    : // Base class initialization
    ::TasMdk::Model(name, description, parent, type_registry)
{
}

NullDefaultsGen::~NullDefaultsGen() {
}

void NullDefaultsGen::Publish(::Smp::IPublication* receiver) {
    // Call base class implementation first
    ::TasMdk::Model::Publish(receiver);

    if (Publish_Callback) {
        Publish_Callback(receiver);
    }

    // Populate the request handlers (only once)
    if (requestHandlers.empty()) {
        PopulateRequestHandlers<NullDefaultsGen>(this, requestHandlers);
    }

    // Publish operation ping
    auto* op_ping = receiver->PublishOperation(
        "ping", // Name
        "", // Description
        ::Smp::ViewKind::VK_All // View Kind
    );
    op_ping->PublishParameter(
        "a", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        Smp::Publication::ParameterDirectionKind::PDK_InOut // Parameter Direction Kind
    );
    op_ping->PublishParameter(
        "b", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        Smp::Publication::ParameterDirectionKind::PDK_In // Parameter Direction Kind
    );
    op_ping->PublishParameter(
        "c", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        Smp::Publication::ParameterDirectionKind::PDK_In // Parameter Direction Kind
    );
    op_ping->PublishParameter(
        "d", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Char8, // Type UUID
        Smp::Publication::ParameterDirectionKind::PDK_In // Parameter Direction Kind
    );
}

void NullDefaultsGen::Invoke(::Smp::IRequest* request) {
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

const ::Smp::Uuid& NullDefaultsGen::GetUuid() const {
    return Uuid_NullDefaults;
}
} // namespace n
