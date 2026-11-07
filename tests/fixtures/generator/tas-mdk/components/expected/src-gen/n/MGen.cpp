// -----------------------------------------------------------------------------
// File Name    : MGen.cpp
// -----------------------------------------------------------------------------
/// @file n/MGen.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <n/M.h>
#include <Smp/Publication/IPublishOperation.h>
#include <Smp/Version.h>
#include <TasMdk/EntryPoint.h>

namespace n
{
::TasMdk::Request::Handler<MGen>::CollectionType MGen::requestHandlers;

void MGen::Configure(::Smp::Services::ILogger* logger, ::Smp::Services::ILinkRegistry* linkRegistry) {
    // Call base implementation first
    ::TasMdk::Model::Configure(logger, linkRegistry);
    if (Configure_Callback) {
        Configure_Callback(logger, linkRegistry);
    }
}

void MGen::Connect(::Smp::ISimulator* simulator) {
    // Call mdk implementation first
    ::TasMdk::Model::Connect(simulator);
    if (Connect_Callback) {
        Connect_Callback(simulator);
    }
}

void MGen::Disconnect() {
    if (Disconnect_Callback) {
        Disconnect_Callback();
    }
    // Call parent implementation last, to remove references to the Simulator and its services
    ::TasMdk::Model::Disconnect();
}

MGen::MGen(
    ::Smp::String8 name,
    ::Smp::String8 description,
    ::Smp::IObject* parent,
    ::Smp::Publication::ITypeRegistry* type_registry)
    : // Base class initialization
    ::TasMdk::Model(name, description, parent, type_registry),
    // EntryPoint: ept_step
    ept_step{ new ::TasMdk::EntryPoint(
        "ept_step", // Name
        "", // Description
        this,
        _entrypoints,
        std::bind(&MGen::_ept_step, this))},
    // Field inp_value
    inp_value{
        "inp_value", // Name
        "", // Description
        this, // Parent
        ::Smp::ViewKind::VK_All, // View Kind
        type_registry, // Type Registry
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        3 // Default value
    }
{
}

MGen::~MGen() {
    delete ept_step;
    ept_step = nullptr;
}

void MGen::Publish(::Smp::IPublication* receiver) {
    // Call base class implementation first
    ::TasMdk::Model::Publish(receiver);

    if (Publish_Callback) {
        Publish_Callback(receiver);
    }

    // Populate the request handlers (only once)
    if (requestHandlers.empty()) {
        PopulateRequestHandlers<MGen>(this, requestHandlers);
    }

    // Publish field inp_value
    receiver->PublishField(&inp_value);

    // Publish operation ope_reset
    receiver->PublishOperation(
        "ope_reset", // Name
        "", // Description
        ::Smp::ViewKind::VK_All // View Kind
    );
}

void MGen::Invoke(::Smp::IRequest* request) {
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

const ::Smp::Uuid& MGen::GetUuid() const {
    return Uuid_M;
}
} // namespace n
