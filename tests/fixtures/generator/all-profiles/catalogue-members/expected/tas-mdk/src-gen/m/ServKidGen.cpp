// Block notice
// line two
// -----------------------------------------------------------------------------
// File Name    : ServKidGen.cpp
// -----------------------------------------------------------------------------
/// @file m/ServKidGen.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <m/ServKid.h>
#include <Smp/Version.h>

namespace m
{
::TasMdk::Request::Handler<ServKidGen>::CollectionType ServKidGen::requestHandlers;

void ServKidGen::Configure(::Smp::Services::ILogger* logger, ::Smp::Services::ILinkRegistry* linkRegistry) {
    // Call base implementation first
    ::m::Serv::Configure(logger, linkRegistry);
    if (Configure_Callback) {
        Configure_Callback(logger, linkRegistry);
    }
}

void ServKidGen::Connect(::Smp::ISimulator* simulator) {
    // Call mdk implementation first
    ::m::Serv::Connect(simulator);
    if (Connect_Callback) {
        Connect_Callback(simulator);
    }
}

void ServKidGen::Disconnect() {
    if (Disconnect_Callback) {
        Disconnect_Callback();
    }
    // Call parent implementation last, to remove references to the Simulator and its services
    ::m::Serv::Disconnect();
}

ServKidGen::ServKidGen(
    ::Smp::String8 name,
    ::Smp::String8 description,
    ::Smp::IObject* parent,
    ::Smp::Publication::ITypeRegistry* type_registry)
    : // Base class initialization
    ::m::Serv(name, description, parent, type_registry)
{
}

ServKidGen::~ServKidGen() {
}

void ServKidGen::Publish(::Smp::IPublication* receiver) {
    // Call base class implementation first
    ::m::Serv::Publish(receiver);

    if (Publish_Callback) {
        Publish_Callback(receiver);
    }

    // Populate the request handlers (only once)
    if (requestHandlers.empty()) {
        PopulateRequestHandlers<ServKidGen>(this, requestHandlers);
    }
}

void ServKidGen::Invoke(::Smp::IRequest* request) {
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
        ::m::Serv::Invoke(request);
    }
}

const ::Smp::Uuid& ServKidGen::GetUuid() const {
    return Uuid_ServKid;
}
} // namespace m
