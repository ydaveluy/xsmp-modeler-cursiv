// -----------------------------------------------------------------------------
// File Name    : SGen.cpp
// -----------------------------------------------------------------------------
/// @file n/SGen.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <esa/ecss/smp/cdk/Request.h>
#include <n/S.h>
#include <Smp/IPublication.h>
#include <Smp/ViewKind.h>

namespace n
{
//--------------------------- Constructor -------------------------
SGen::SGen(
    ::Smp::String8 name,
    ::Smp::String8 description,
    ::Smp::IComposite* parent,
    ::Smp::ISimulator* simulator):
    // Base class
    ::esa::ecss::smp::cdk::Service(name, description, parent, simulator) {
}

/// Virtual destructor that is called by inherited classes as well.
SGen::~SGen() {
}

void SGen::Publish(::Smp::IPublication* receiver) {
    // Call parent class implementation first
    ::esa::ecss::smp::cdk::Service::Publish(receiver);

    if (requestHandlers.empty()) {
        PopulateRequestHandlers<SGen>(this, requestHandlers);
    }

    // Publish operation h
    receiver->PublishOperation(
        "h", // Name
        "", // Description
        ::Smp::ViewKind::VK_All // View Kind
    );

    dynamic_cast<::n::S*>(this)->DoPublish(receiver);
}

void SGen::Configure(::Smp::Services::ILogger* logger, ::Smp::Services::ILinkRegistry* linkRegistry) {
    // Call parent implementation first
    ::esa::ecss::smp::cdk::Service::Configure(logger, linkRegistry);

    dynamic_cast<::n::S*>(this)->DoConfigure(logger, linkRegistry);
}

void SGen::Connect(::Smp::ISimulator* simulator) {
    // Call parent implementation first
    ::esa::ecss::smp::cdk::Service::Connect(simulator);

    dynamic_cast<::n::S*>(this)->DoConnect(simulator);
}

void SGen::Disconnect() {
    if (this->GetState() == ::Smp::ComponentStateKind::CSK_Connected) {
        dynamic_cast<::n::S*>(this)->DoDisconnect();
    }

    // Call parent implementation last, to remove references to the Simulator and its services
    ::esa::ecss::smp::cdk::Service::Disconnect();
}

::esa::ecss::smp::cdk::RequestContainer<SGen>::Map SGen::requestHandlers;

void SGen::Invoke(::Smp::IRequest* request) {
    if (!request) {
        return;
    }
    auto it = requestHandlers.find(request->GetOperationName());
    if (it != requestHandlers.end()) {
        it->second->Execute(*this, request);
    } else {
        // pass the request down to the base class
        ::esa::ecss::smp::cdk::Service::Invoke(request);
    }
}

const ::Smp::Uuid& SGen::GetUuid() const {
    return Uuid_S;
}
} // namespace n
