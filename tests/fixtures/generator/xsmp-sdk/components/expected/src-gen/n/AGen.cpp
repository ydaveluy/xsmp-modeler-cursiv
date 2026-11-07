// -----------------------------------------------------------------------------
// File Name    : AGen.cpp
// -----------------------------------------------------------------------------
/// @file n/AGen.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <n/A.h>
#include <Smp/ViewKind.h>
#include <string>
#include <Xsmp/ComponentHelper.h>
#include <Xsmp/Request.h>

#if defined(__has_include)
#if __has_include(<Smp/Version.h>)
#include <Smp/Version.h>
#endif
#endif

#ifndef ECSS_SMP_VERSION
#define ECSS_SMP_VERSION 202003L
#endif

namespace n
{
//--------------------------- Constructor -------------------------
AGen::AGen(
    ::Smp::String8 name,
    ::Smp::String8 description,
    ::Smp::IComposite* parent,
    ::Smp::ISimulator* simulator):
    // Base class
    ::Xsmp::Model(name, description, parent, simulator) { }

void AGen::Publish(::Smp::IPublication* receiver) {
    // Call parent class implementation first
    ::Xsmp::Model::Publish(receiver);

    // Publish operation f
    receiver->PublishOperation(
        "f", // Name
        "", // Description
        ::Smp::ViewKind::VK_All // View Kind
    );

    // Call user DoPublish if any
    ::Xsmp::Component::Helper::Publish<::n::A>(this, receiver);
}

void AGen::Configure(::Smp::Services::ILogger* logger, ::Smp::Services::ILinkRegistry* linkRegistry) {
    // Call parent implementation first
    ::Xsmp::Model::Configure(logger, linkRegistry);

    // Call user DoConfigure if any
    ::Xsmp::Component::Helper::Configure<::n::A>(this, logger, linkRegistry);
}

void AGen::Connect(::Smp::ISimulator* simulator) {
    // Call parent implementation first
    ::Xsmp::Model::Connect(simulator);

    // Call user DoConnect if any
    ::Xsmp::Component::Helper::Connect<::n::A>(this, simulator);
}

void AGen::Disconnect() {
    if (this->::Xsmp::Component::GetState() == ::Smp::ComponentStateKind::CSK_Connected) {
        // Call user DoDisconnect if any
        ::Xsmp::Component::Helper::Disconnect<::n::A>(this);
    }

    // Call parent implementation last, to remove references to the Simulator and its services
    ::Xsmp::Model::Disconnect();
}

std::map<std::string_view, std::function<void(AGen*, ::Smp::IRequest*)>> AGen::_requestHandlers{
    // Handler for Operation f
    {"f",
    [](AGen* component, ::Smp::IRequest *) {
        component->f();
    }},
};

void AGen::Invoke(::Smp::IRequest* request) {
    if (!request) {
        return;
    }
#if ECSS_SMP_VERSION >= 202503L
    // the request carries the bare property name and tells a getter
    // from a setter apart through its type
    const std::string key =
            (request->GetType() == ::Smp::RequestType::RT_Get ? "get_"
             : request->GetType() == ::Smp::RequestType::RT_Set ? "set_"
             : "") + std::string{request->GetName()};
#else
    // the request already names the property accessor
    const std::string_view key{request->GetOperationName()};
#endif
    if (auto it = _requestHandlers.find(key);
            it != _requestHandlers.end()) {
        it->second(this, request);
    } else {
        // pass the request down to the base class
        ::Xsmp::Model::Invoke(request);
    }
}

const ::Smp::Uuid& AGen::GetUuid() const {
    return Uuid_A;
}
} // namespace n
