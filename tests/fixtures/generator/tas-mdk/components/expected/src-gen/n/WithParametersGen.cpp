// -----------------------------------------------------------------------------
// File Name    : WithParametersGen.cpp
// -----------------------------------------------------------------------------
/// @file n/WithParametersGen.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <n/WithParameters.h>
#include <Smp/Publication/IPublishOperation.h>
#include <Smp/Version.h>

namespace n
{
::TasMdk::Request::Handler<WithParametersGen>::CollectionType WithParametersGen::requestHandlers;

void WithParametersGen::Configure(::Smp::Services::ILogger* logger, ::Smp::Services::ILinkRegistry* linkRegistry) {
    // Call base implementation first
    ::TasMdk::Model::Configure(logger, linkRegistry);
    if (Configure_Callback) {
        Configure_Callback(logger, linkRegistry);
    }
}

void WithParametersGen::Connect(::Smp::ISimulator* simulator) {
    // Call mdk implementation first
    ::TasMdk::Model::Connect(simulator);
    if (Connect_Callback) {
        Connect_Callback(simulator);
    }
}

void WithParametersGen::Disconnect() {
    if (Disconnect_Callback) {
        Disconnect_Callback();
    }
    // Call parent implementation last, to remove references to the Simulator and its services
    ::TasMdk::Model::Disconnect();
}

WithParametersGen::WithParametersGen(
    ::Smp::String8 name,
    ::Smp::String8 description,
    ::Smp::IObject* parent,
    ::Smp::Publication::ITypeRegistry* type_registry)
    : // Base class initialization
    ::TasMdk::Model(name, description, parent, type_registry)
{
}

WithParametersGen::~WithParametersGen() {
}

void WithParametersGen::Publish(::Smp::IPublication* receiver) {
    // Call base class implementation first
    ::TasMdk::Model::Publish(receiver);

    if (Publish_Callback) {
        Publish_Callback(receiver);
    }

    // Populate the request handlers (only once)
    if (requestHandlers.empty()) {
        PopulateRequestHandlers<WithParametersGen>(this, requestHandlers);
    }

    // Publish operation ping
    auto* op_ping = receiver->PublishOperation(
        "ping", // Name
        "", // Description
        ::Smp::ViewKind::VK_All // View Kind
    );
    op_ping->PublishParameter(
        "count", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        Smp::Publication::ParameterDirectionKind::PDK_In // Parameter Direction Kind
    );
}

void WithParametersGen::Invoke(::Smp::IRequest* request) {
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

const ::Smp::Uuid& WithParametersGen::GetUuid() const {
    return Uuid_WithParameters;
}
} // namespace n
