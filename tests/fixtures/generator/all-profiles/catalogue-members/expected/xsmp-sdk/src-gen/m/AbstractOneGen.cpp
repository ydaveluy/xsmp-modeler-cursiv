// Block notice
// line two
// -----------------------------------------------------------------------------
// File Name    : AbstractOneGen.cpp
// -----------------------------------------------------------------------------
/// @file m/AbstractOneGen.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <m/AbstractOne.h>
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

namespace m
{
//--------------------------- Constructor -------------------------
AbstractOneGen::AbstractOneGen(
    ::Smp::String8 name,
    ::Smp::String8 description,
    ::Smp::IComposite* parent,
    ::Smp::ISimulator* simulator):
    // Base class
    ::Xsmp::Model(name, description, parent, simulator),
    // Field connectedState
    connectedState {false},
    // Field valueState
    valueState {} { }

void AbstractOneGen::Publish(::Smp::IPublication* receiver) {
    // Call parent class implementation first
    ::Xsmp::Model::Publish(receiver);

    // Publish field connectedState
    receiver->PublishField(
        "connectedState", // Name
        "", // Description
        &connectedState, // Address
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        false // Output
    );

    // Publish field valueState
    receiver->PublishField(
        "valueState", // Name
        "", // Description
        &valueState, // Address
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        false // Output
    );

    // Publish operation mustImplement
    auto* op_mustImplement = receiver->PublishOperation(
        "mustImplement", // Name
        "", // Description
        ::Smp::ViewKind::VK_All // View Kind
    );
    op_mustImplement->PublishParameter(
        "return", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        Smp::Publication::ParameterDirectionKind::PDK_Return // Parameter Direction Kind
    );

    // Publish operation ping
    auto* op_ping = receiver->PublishOperation(
        "ping", // Name
        "", // Description
        ::Smp::ViewKind::VK_All // View Kind
    );
    op_ping->PublishParameter(
        "value", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        Smp::Publication::ParameterDirectionKind::PDK_In // Parameter Direction Kind
    );
    op_ping->PublishParameter(
        "return", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Bool, // Type UUID
        Smp::Publication::ParameterDirectionKind::PDK_Return // Parameter Direction Kind
    );

    // Publish Property connected
    receiver->PublishProperty(
        "connected", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Bool, // Type UUID
        ::Smp::AccessKind::AK_ReadWrite, // Access Kind
        ::Smp::ViewKind::VK_All // View Kind
    );

    // Publish Property ro
    receiver->PublishProperty(
        "ro", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        ::Smp::AccessKind::AK_ReadOnly, // Access Kind
        ::Smp::ViewKind::VK_All // View Kind
    );

    // Publish Property wo
    receiver->PublishProperty(
        "wo", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        ::Smp::AccessKind::AK_WriteOnly, // Access Kind
        ::Smp::ViewKind::VK_All // View Kind
    );

    // Call user DoPublish if any
    ::Xsmp::Component::Helper::Publish<::m::AbstractOne>(this, receiver);
}

void AbstractOneGen::Configure(::Smp::Services::ILogger* logger, ::Smp::Services::ILinkRegistry* linkRegistry) {
    // Call parent implementation first
    ::Xsmp::Model::Configure(logger, linkRegistry);

    // Call user DoConfigure if any
    ::Xsmp::Component::Helper::Configure<::m::AbstractOne>(this, logger, linkRegistry);
}

void AbstractOneGen::Connect(::Smp::ISimulator* simulator) {
    // Call parent implementation first
    ::Xsmp::Model::Connect(simulator);

    // Call user DoConnect if any
    ::Xsmp::Component::Helper::Connect<::m::AbstractOne>(this, simulator);
}

void AbstractOneGen::Disconnect() {
    if (this->::Xsmp::Component::GetState() == ::Smp::ComponentStateKind::CSK_Connected) {
        // Call user DoDisconnect if any
        ::Xsmp::Component::Helper::Disconnect<::m::AbstractOne>(this);
    }

    // Call parent implementation last, to remove references to the Simulator and its services
    ::Xsmp::Model::Disconnect();
}

std::map<std::string_view, std::function<void(AbstractOneGen*, ::Smp::IRequest*)>> AbstractOneGen::_requestHandlers{
    // Handler for Operation mustImplement
    {"mustImplement",
    [](AbstractOneGen* component, ::Smp::IRequest *request) {
        ::Xsmp::Request::setReturnValue(request, ::Smp::PrimitiveTypeKind::PTK_Int32, component->mustImplement());
    }},

    // Handler for Operation ping
    {"ping",
    [](AbstractOneGen* component, ::Smp::IRequest *request) {
        auto p_value = ::Xsmp::Request::get<::Smp::Int32>(component, request, "value", ::Smp::PrimitiveTypeKind::PTK_Int32);

        ::Xsmp::Request::setReturnValue(request, ::Smp::PrimitiveTypeKind::PTK_Bool, component->ping(p_value));
    }},

    // Getter handler for Property connected
    {"get_connected",
    [](AbstractOneGen* component, ::Smp::IRequest *request) {
        ::Xsmp::Request::setReturnValue(request, ::Smp::PrimitiveTypeKind::PTK_Bool, component->get_connected());
    }},

    // Setter handler for Property connected
    {"set_connected",
    [](AbstractOneGen* component, ::Smp::IRequest *request) {
        component->set_connected(::Xsmp::Request::get<::Smp::Bool>(component, request, "connected", ::Smp::PrimitiveTypeKind::PTK_Bool));
    }},

    // Getter handler for Property ro
    {"get_ro",
    [](AbstractOneGen* component, ::Smp::IRequest *request) {
        ::Xsmp::Request::setReturnValue(request, ::Smp::PrimitiveTypeKind::PTK_Int32, component->get_ro());
    }},

    // Setter handler for Property wo
    {"set_wo",
    [](AbstractOneGen* component, ::Smp::IRequest *request) {
        component->set_wo(::Xsmp::Request::get<::Smp::Int32>(component, request, "wo", ::Smp::PrimitiveTypeKind::PTK_Int32));
    }},
};

void AbstractOneGen::Invoke(::Smp::IRequest* request) {
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

const ::Smp::Uuid& AbstractOneGen::GetUuid() const {
    return Uuid_AbstractOne;
}
::Smp::Bool AbstractOneGen::get_connected(){
    return this->connectedState;
}
void AbstractOneGen::set_connected(::Smp::Bool value) {
    this->connectedState = value;
}

::Smp::Int32 AbstractOneGen::get_ro(){
    return this->valueState;
}

void AbstractOneGen::set_wo(::Smp::Int32 value) {
    this->valueState = value;
}
} // namespace m
