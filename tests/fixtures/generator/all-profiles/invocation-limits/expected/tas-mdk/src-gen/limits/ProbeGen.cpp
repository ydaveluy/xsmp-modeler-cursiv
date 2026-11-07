// -----------------------------------------------------------------------------
// File Name    : ProbeGen.cpp
// -----------------------------------------------------------------------------
/// @file limits/ProbeGen.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <limits/Probe.h>
#include <Smp/Publication/IPublishOperation.h>
#include <Smp/Version.h>

namespace limits
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

    // WARNING: Operation make is not invokable: constructors cannot be dynamically invoked.

    // WARNING: Operation create is not invokable: constructors cannot be dynamically invoked.

    // WARNING: Operation pointer is not invokable: pointer return values cannot be represented by Smp::AnySimple.

    // WARNING: Operation calculate is not invokable: overloaded operation names cannot be distinguished by Smp::IRequest.

    // WARNING: Operation calculate is not invokable: overloaded operation names cannot be distinguished by Smp::IRequest.

    // Publish operation normal
    auto* op_normal = receiver->PublishOperation(
        "normal", // Name
        "", // Description
        ::Smp::ViewKind::VK_All // View Kind
    );
    op_normal->PublishParameter(
        "value", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        Smp::Publication::ParameterDirectionKind::PDK_In // Parameter Direction Kind
    );
    op_normal->PublishParameter(
        "return", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        Smp::Publication::ParameterDirectionKind::PDK_Return // Parameter Direction Kind
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
} // namespace limits
