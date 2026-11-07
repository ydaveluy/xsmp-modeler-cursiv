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
MGen::MGen(
    ::Smp::String8 name,
    ::Smp::String8 description,
    ::Smp::IComposite* parent,
    ::Smp::ISimulator* simulator):
    // Base class
    ::Xsmp::Model(name, description, parent, simulator) { }

void MGen::Publish(::Smp::IPublication* receiver) {
    // Call parent class implementation first
    ::Xsmp::Model::Publish(receiver);

    // Publish operation shared
    receiver->PublishOperation(
        "shared", // Name
        "", // Description
        ::Smp::ViewKind::VK_All // View Kind
    );

    // Publish operation plain
    auto* op_plain = receiver->PublishOperation(
        "plain", // Name
        "", // Description
        ::Smp::ViewKind::VK_All // View Kind
    );
    op_plain->PublishParameter(
        "return", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        Smp::Publication::ParameterDirectionKind::PDK_Return // Parameter Direction Kind
    );

    // WARNING: Operation self is not invokable: pointer return values cannot be represented by Smp::AnySimple.

    // Publish operation overridden
    receiver->PublishOperation(
        "overridden", // Name
        "", // Description
        ::Smp::ViewKind::VK_All // View Kind
    );

    // WARNING: Operation fluent is not invokable: pointer return values cannot be represented by Smp::AnySimple.

    // Publish Property sharedValue
    receiver->PublishProperty(
        "sharedValue", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        ::Smp::AccessKind::AK_ReadWrite, // Access Kind
        ::Smp::ViewKind::VK_All // View Kind
    );

    // Publish Property plainValue
    receiver->PublishProperty(
        "plainValue", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        ::Smp::AccessKind::AK_ReadWrite, // Access Kind
        ::Smp::ViewKind::VK_All // View Kind
    );

    // Call user DoPublish if any
    ::Xsmp::Component::Helper::Publish<::n::M>(this, receiver);
}

void MGen::Configure(::Smp::Services::ILogger* logger, ::Smp::Services::ILinkRegistry* linkRegistry) {
    // Call parent implementation first
    ::Xsmp::Model::Configure(logger, linkRegistry);

    // Call user DoConfigure if any
    ::Xsmp::Component::Helper::Configure<::n::M>(this, logger, linkRegistry);
}

void MGen::Connect(::Smp::ISimulator* simulator) {
    // Call parent implementation first
    ::Xsmp::Model::Connect(simulator);

    // Call user DoConnect if any
    ::Xsmp::Component::Helper::Connect<::n::M>(this, simulator);
}

void MGen::Disconnect() {
    if (this->::Xsmp::Component::GetState() == ::Smp::ComponentStateKind::CSK_Connected) {
        // Call user DoDisconnect if any
        ::Xsmp::Component::Helper::Disconnect<::n::M>(this);
    }

    // Call parent implementation last, to remove references to the Simulator and its services
    ::Xsmp::Model::Disconnect();
}

std::map<std::string_view, std::function<void(MGen*, ::Smp::IRequest*)>> MGen::_requestHandlers{
    // Handler for Operation shared
    {"shared",
    [](MGen* component, ::Smp::IRequest *) {
        component->shared();
    }},

    // Handler for Operation plain
    {"plain",
    [](MGen* component, ::Smp::IRequest *request) {
        ::Xsmp::Request::setReturnValue(request, ::Smp::PrimitiveTypeKind::PTK_Int32, component->plain());
    }},

    // Handler for Operation overridden
    {"overridden",
    [](MGen* component, ::Smp::IRequest *) {
        component->overridden();
    }},

    // Getter handler for Property sharedValue
    {"get_sharedValue",
    [](MGen* component, ::Smp::IRequest *request) {
        ::Xsmp::Request::setReturnValue(request, ::Smp::PrimitiveTypeKind::PTK_Int32, component->get_sharedValue());
    }},

    // Setter handler for Property sharedValue
    {"set_sharedValue",
    [](MGen* component, ::Smp::IRequest *request) {
        component->set_sharedValue(::Xsmp::Request::get<::Smp::Int32>(component, request, "sharedValue", ::Smp::PrimitiveTypeKind::PTK_Int32));
    }},

    // Getter handler for Property plainValue
    {"get_plainValue",
    [](MGen* component, ::Smp::IRequest *request) {
        ::Xsmp::Request::setReturnValue(request, ::Smp::PrimitiveTypeKind::PTK_Int32, component->get_plainValue());
    }},

    // Setter handler for Property plainValue
    {"set_plainValue",
    [](MGen* component, ::Smp::IRequest *request) {
        component->set_plainValue(::Xsmp::Request::get<::Smp::Int32>(component, request, "plainValue", ::Smp::PrimitiveTypeKind::PTK_Int32));
    }},
};

void MGen::Invoke(::Smp::IRequest* request) {
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

const ::Smp::Uuid& MGen::GetUuid() const {
    return Uuid_M;
}
} // namespace n
