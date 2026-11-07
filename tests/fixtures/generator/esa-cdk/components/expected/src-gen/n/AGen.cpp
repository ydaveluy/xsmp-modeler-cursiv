// -----------------------------------------------------------------------------
// File Name    : AGen.cpp
// -----------------------------------------------------------------------------
/// @file n/AGen.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <esa/ecss/smp/cdk/Request.h>
#include <n/A.h>
#include <Smp/IPublication.h>
#include <Smp/ViewKind.h>

namespace n
{
//--------------------------- Constructor -------------------------
AGen::AGen(
    ::Smp::String8 name,
    ::Smp::String8 description,
    ::Smp::IComposite* parent,
    ::Smp::ISimulator* simulator):
    // Base class
    ::esa::ecss::smp::cdk::Model(name, description, parent, simulator) {
}

/// Virtual destructor that is called by inherited classes as well.
AGen::~AGen() {
}

void AGen::Publish(::Smp::IPublication* receiver) {
    // Call parent class implementation first
    ::esa::ecss::smp::cdk::Model::Publish(receiver);

    if (requestHandlers.empty()) {
        PopulateRequestHandlers<AGen>(this, requestHandlers);
    }

    // Publish operation f
    receiver->PublishOperation(
        "f", // Name
        "", // Description
        ::Smp::ViewKind::VK_All // View Kind
    );

    dynamic_cast<::n::A*>(this)->DoPublish(receiver);
}

void AGen::Configure(::Smp::Services::ILogger* logger, ::Smp::Services::ILinkRegistry* linkRegistry) {
    // Call parent implementation first
    ::esa::ecss::smp::cdk::Model::Configure(logger, linkRegistry);

    dynamic_cast<::n::A*>(this)->DoConfigure(logger, linkRegistry);
}

void AGen::Connect(::Smp::ISimulator* simulator) {
    // Call parent implementation first
    ::esa::ecss::smp::cdk::Model::Connect(simulator);

    dynamic_cast<::n::A*>(this)->DoConnect(simulator);
}

void AGen::Disconnect() {
    if (this->GetState() == ::Smp::ComponentStateKind::CSK_Connected) {
        dynamic_cast<::n::A*>(this)->DoDisconnect();
    }

    // Call parent implementation last, to remove references to the Simulator and its services
    ::esa::ecss::smp::cdk::Model::Disconnect();
}

::esa::ecss::smp::cdk::RequestContainer<AGen>::Map AGen::requestHandlers;

void AGen::Invoke(::Smp::IRequest* request) {
    if (!request) {
        return;
    }
    auto it = requestHandlers.find(request->GetOperationName());
    if (it != requestHandlers.end()) {
        it->second->Execute(*this, request);
    } else {
        // pass the request down to the base class
        ::esa::ecss::smp::cdk::Model::Invoke(request);
    }
}

const ::Smp::Uuid& AGen::GetUuid() const {
    return Uuid_A;
}
} // namespace n
