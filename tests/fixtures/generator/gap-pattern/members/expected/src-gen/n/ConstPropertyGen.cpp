// -----------------------------------------------------------------------------
// File Name    : ConstPropertyGen.cpp
// -----------------------------------------------------------------------------
/// @file n/ConstPropertyGen.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <n/ConstProperty.h>
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
ConstPropertyGen::ConstPropertyGen(
    ::Smp::String8 name,
    ::Smp::String8 description,
    ::Smp::IComposite* parent,
    ::Smp::ISimulator* simulator):
    // Base class
    ::Xsmp::Model(name, description, parent, simulator) { }

void ConstPropertyGen::Publish(::Smp::IPublication* receiver) {
    // Call parent class implementation first
    ::Xsmp::Model::Publish(receiver);

    // Publish Property p
    receiver->PublishProperty(
        "p", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        ::Smp::AccessKind::AK_ReadWrite, // Access Kind
        ::Smp::ViewKind::VK_All // View Kind
    );

    // Call user DoPublish if any
    ::Xsmp::Component::Helper::Publish<::n::ConstProperty>(this, receiver);
}

void ConstPropertyGen::Configure(::Smp::Services::ILogger* logger, ::Smp::Services::ILinkRegistry* linkRegistry) {
    // Call parent implementation first
    ::Xsmp::Model::Configure(logger, linkRegistry);

    // Call user DoConfigure if any
    ::Xsmp::Component::Helper::Configure<::n::ConstProperty>(this, logger, linkRegistry);
}

void ConstPropertyGen::Connect(::Smp::ISimulator* simulator) {
    // Call parent implementation first
    ::Xsmp::Model::Connect(simulator);

    // Call user DoConnect if any
    ::Xsmp::Component::Helper::Connect<::n::ConstProperty>(this, simulator);
}

void ConstPropertyGen::Disconnect() {
    if (this->::Xsmp::Component::GetState() == ::Smp::ComponentStateKind::CSK_Connected) {
        // Call user DoDisconnect if any
        ::Xsmp::Component::Helper::Disconnect<::n::ConstProperty>(this);
    }

    // Call parent implementation last, to remove references to the Simulator and its services
    ::Xsmp::Model::Disconnect();
}

std::map<std::string_view, std::function<void(ConstPropertyGen*, ::Smp::IRequest*)>> ConstPropertyGen::_requestHandlers{
    // Getter handler for Property p
    {"get_p",
    [](ConstPropertyGen* component, ::Smp::IRequest *request) {
        ::Xsmp::Request::setReturnValue(request, ::Smp::PrimitiveTypeKind::PTK_Int32, component->get_p());
    }},

    // Setter handler for Property p
    {"set_p",
    [](ConstPropertyGen* component, ::Smp::IRequest *request) {
        component->set_p(::Xsmp::Request::get<::Smp::Int32>(component, request, "p", ::Smp::PrimitiveTypeKind::PTK_Int32));
    }},
};

void ConstPropertyGen::Invoke(::Smp::IRequest* request) {
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

const ::Smp::Uuid& ConstPropertyGen::GetUuid() const {
    return Uuid_ConstProperty;
}
} // namespace n
