// -----------------------------------------------------------------------------
// File Name    : ChildGen.cpp
// -----------------------------------------------------------------------------
/// @file limits/ChildGen.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <limits/Child.h>
#include <Smp/Publication/IPublishOperation.h>
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

namespace limits
{
//--------------------------- Constructor -------------------------
ChildGen::ChildGen(
    ::Smp::String8 name,
    ::Smp::String8 description,
    ::Smp::IComposite* parent,
    ::Smp::ISimulator* simulator):
    // Base class
    ::limits::Probe(name, description, parent, simulator) { }

void ChildGen::Publish(::Smp::IPublication* receiver) {
    // Call parent class implementation first
    ::limits::Probe::Publish(receiver);

    // WARNING: Operation normal is not invokable: overloaded operation names cannot be distinguished by Smp::IRequest.

    // Publish operation fresh
    auto* op_fresh = receiver->PublishOperation(
        "fresh", // Name
        "", // Description
        ::Smp::ViewKind::VK_All // View Kind
    );
    op_fresh->PublishParameter(
        "return", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        Smp::Publication::ParameterDirectionKind::PDK_Return // Parameter Direction Kind
    );

    // Call user DoPublish if any
    ::Xsmp::Component::Helper::Publish<::limits::Child>(this, receiver);
}

void ChildGen::Configure(::Smp::Services::ILogger* logger, ::Smp::Services::ILinkRegistry* linkRegistry) {
    // Call parent implementation first
    ::limits::Probe::Configure(logger, linkRegistry);

    // Call user DoConfigure if any
    ::Xsmp::Component::Helper::Configure<::limits::Child>(this, logger, linkRegistry);
}

void ChildGen::Connect(::Smp::ISimulator* simulator) {
    // Call parent implementation first
    ::limits::Probe::Connect(simulator);

    // Call user DoConnect if any
    ::Xsmp::Component::Helper::Connect<::limits::Child>(this, simulator);
}

void ChildGen::Disconnect() {
    if (this->::Xsmp::Component::GetState() == ::Smp::ComponentStateKind::CSK_Connected) {
        // Call user DoDisconnect if any
        ::Xsmp::Component::Helper::Disconnect<::limits::Child>(this);
    }

    // Call parent implementation last, to remove references to the Simulator and its services
    ::limits::Probe::Disconnect();
}

std::map<std::string_view, std::function<void(ChildGen*, ::Smp::IRequest*)>> ChildGen::_requestHandlers{
    // Handler for Operation fresh
    {"fresh",
    [](ChildGen* component, ::Smp::IRequest *request) {
        ::Xsmp::Request::setReturnValue(request, ::Smp::PrimitiveTypeKind::PTK_Int32, component->fresh());
    }},
};

void ChildGen::Invoke(::Smp::IRequest* request) {
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
        ::limits::Probe::Invoke(request);
    }
}

const ::Smp::Uuid& ChildGen::GetUuid() const {
    return Uuid_Child;
}
} // namespace limits
