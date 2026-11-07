// -----------------------------------------------------------------------------
// File Name    : BGen.cpp
// -----------------------------------------------------------------------------
/// @file n/BGen.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <esa/ecss/smp/cdk/Request.h>
#include <n/B.h>
#include <Smp/IPublication.h>
#include <Smp/ViewKind.h>

namespace n
{
//--------------------------- Constructor -------------------------
BGen::BGen(
    ::Smp::String8 name,
    ::Smp::String8 description,
    ::Smp::IComposite* parent,
    ::Smp::ISimulator* simulator):
    // Base class
    ::n::A(name, description, parent, simulator) {
}

/// Virtual destructor that is called by inherited classes as well.
BGen::~BGen() {
}

void BGen::Publish(::Smp::IPublication* receiver) {
    // Call parent class implementation first
    ::n::A::Publish(receiver);

    if (requestHandlers.empty()) {
        PopulateRequestHandlers<BGen>(this, requestHandlers);
    }

    // Publish operation g
    receiver->PublishOperation(
        "g", // Name
        "", // Description
        ::Smp::ViewKind::VK_All // View Kind
    );

    // WARNING: Operation shared is not invokable: it is static.

    dynamic_cast<::n::B*>(this)->DoPublish(receiver);
}

void BGen::Configure(::Smp::Services::ILogger* logger, ::Smp::Services::ILinkRegistry* linkRegistry) {
    // Call parent implementation first
    ::n::A::Configure(logger, linkRegistry);

    dynamic_cast<::n::B*>(this)->DoConfigure(logger, linkRegistry);
}

void BGen::Connect(::Smp::ISimulator* simulator) {
    // Call parent implementation first
    ::n::A::Connect(simulator);

    dynamic_cast<::n::B*>(this)->DoConnect(simulator);
}

void BGen::Disconnect() {
    if (this->GetState() == ::Smp::ComponentStateKind::CSK_Connected) {
        dynamic_cast<::n::B*>(this)->DoDisconnect();
    }

    // Call parent implementation last, to remove references to the Simulator and its services
    ::n::A::Disconnect();
}

::esa::ecss::smp::cdk::RequestContainer<BGen>::Map BGen::requestHandlers;

void BGen::Invoke(::Smp::IRequest* request) {
    if (!request) {
        return;
    }
    auto it = requestHandlers.find(request->GetOperationName());
    if (it != requestHandlers.end()) {
        it->second->Execute(*this, request);
    } else {
        // pass the request down to the base class
        ::n::A::Invoke(request);
    }
}

const ::Smp::Uuid& BGen::GetUuid() const {
    return Uuid_B;
}
} // namespace n
