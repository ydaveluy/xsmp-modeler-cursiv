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

    // WARNING: Operation huge is not invokable: its return type is not a SimpleType.

    // WARNING: Operation structure is not invokable: its return type is not a SimpleType.

    // Publish operation literal
    auto* op_literal = receiver->PublishOperation(
        "literal", // Name
        "", // Description
        ::Smp::ViewKind::VK_All // View Kind
    );
    op_literal->PublishParameter(
        "return", // Name
        "", // Description
        ::n::Uuid_E, // Type UUID
        Smp::Publication::ParameterDirectionKind::PDK_Return // Parameter Direction Kind
    );

    // Publish operation number
    auto* op_number = receiver->PublishOperation(
        "number", // Name
        "", // Description
        ::Smp::ViewKind::VK_All // View Kind
    );
    op_number->PublishParameter(
        "return", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        Smp::Publication::ParameterDirectionKind::PDK_Return // Parameter Direction Kind
    );

    // Publish operation reference
    auto* op_reference = receiver->PublishOperation(
        "reference", // Name
        "", // Description
        ::Smp::ViewKind::VK_All // View Kind
    );
    op_reference->PublishParameter(
        "return", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        Smp::Publication::ParameterDirectionKind::PDK_Return // Parameter Direction Kind
    );

    // Publish Property value
    receiver->PublishProperty(
        "value", // Name
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
    // Handler for Operation literal
    {"literal",
    [](MGen* component, ::Smp::IRequest *request) {
        ::Xsmp::Request::setReturnValue(request, ::Smp::PrimitiveTypeKind::PTK_Int32, component->literal());
    }},

    // Handler for Operation number
    {"number",
    [](MGen* component, ::Smp::IRequest *request) {
        ::Xsmp::Request::setReturnValue(request, ::Smp::PrimitiveTypeKind::PTK_Int32, component->number());
    }},

    // Handler for Operation reference
    {"reference",
    [](MGen* component, ::Smp::IRequest *request) {
        ::Xsmp::Request::setReturnValue(request, ::Smp::PrimitiveTypeKind::PTK_Int32, component->reference());
    }},

    // Getter handler for Property value
    {"get_value",
    [](MGen* component, ::Smp::IRequest *request) {
        ::Xsmp::Request::setReturnValue(request, ::Smp::PrimitiveTypeKind::PTK_Int32, component->get_value());
    }},

    // Setter handler for Property value
    {"set_value",
    [](MGen* component, ::Smp::IRequest *request) {
        auto value = ::Xsmp::Request::get<::Smp::Int32>(component, request, "value", ::Smp::PrimitiveTypeKind::PTK_Int32);
        component->set_value(value);
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
