// -----------------------------------------------------------------------------
// File Name    : PointersGen.cpp
// -----------------------------------------------------------------------------
/// @file n/PointersGen.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <n/Pointers.h>
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
PointersGen::PointersGen(
    ::Smp::String8 name,
    ::Smp::String8 description,
    ::Smp::IComposite* parent,
    ::Smp::ISimulator* simulator):
    // Base class
    ::Xsmp::Model(name, description, parent, simulator) { }

void PointersGen::Publish(::Smp::IPublication* receiver) {
    // Call parent class implementation first
    ::Xsmp::Model::Publish(receiver);

    // WARNING: Property pointer is not invokable: pointer properties cannot be represented by Smp::AnySimple.

    // Publish Property reference
    receiver->PublishProperty(
        "reference", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        ::Smp::AccessKind::AK_ReadWrite, // Access Kind
        ::Smp::ViewKind::VK_All // View Kind
    );

    // Publish Property plain
    receiver->PublishProperty(
        "plain", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        ::Smp::AccessKind::AK_ReadWrite, // Access Kind
        ::Smp::ViewKind::VK_All // View Kind
    );

    // WARNING: Operation address is not invokable: pointer return values cannot be represented by Smp::AnySimple.

    // Publish operation alias
    auto* op_alias = receiver->PublishOperation(
        "alias", // Name
        "", // Description
        ::Smp::ViewKind::VK_All // View Kind
    );
    op_alias->PublishParameter(
        "return", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        Smp::Publication::ParameterDirectionKind::PDK_Return // Parameter Direction Kind
    );

    // Call user DoPublish if any
    ::Xsmp::Component::Helper::Publish<::n::Pointers>(this, receiver);
}

void PointersGen::Configure(::Smp::Services::ILogger* logger, ::Smp::Services::ILinkRegistry* linkRegistry) {
    // Call parent implementation first
    ::Xsmp::Model::Configure(logger, linkRegistry);

    // Call user DoConfigure if any
    ::Xsmp::Component::Helper::Configure<::n::Pointers>(this, logger, linkRegistry);
}

void PointersGen::Connect(::Smp::ISimulator* simulator) {
    // Call parent implementation first
    ::Xsmp::Model::Connect(simulator);

    // Call user DoConnect if any
    ::Xsmp::Component::Helper::Connect<::n::Pointers>(this, simulator);
}

void PointersGen::Disconnect() {
    if (this->::Xsmp::Component::GetState() == ::Smp::ComponentStateKind::CSK_Connected) {
        // Call user DoDisconnect if any
        ::Xsmp::Component::Helper::Disconnect<::n::Pointers>(this);
    }

    // Call parent implementation last, to remove references to the Simulator and its services
    ::Xsmp::Model::Disconnect();
}

std::map<std::string_view, std::function<void(PointersGen*, ::Smp::IRequest*)>> PointersGen::_requestHandlers{
    // Handler for Operation alias
    {"alias",
    [](PointersGen* component, ::Smp::IRequest *request) {
        ::Xsmp::Request::setReturnValue(request, ::Smp::PrimitiveTypeKind::PTK_Int32, component->alias());
    }},

    // Getter handler for Property reference
    {"get_reference",
    [](PointersGen* component, ::Smp::IRequest *request) {
        ::Xsmp::Request::setReturnValue(request, ::Smp::PrimitiveTypeKind::PTK_Int32, component->get_reference());
    }},

    // Setter handler for Property reference
    {"set_reference",
    [](PointersGen* component, ::Smp::IRequest *request) {
        auto value = ::Xsmp::Request::get<::Smp::Int32>(component, request, "reference", ::Smp::PrimitiveTypeKind::PTK_Int32);
        component->set_reference(value);
    }},

    // Getter handler for Property plain
    {"get_plain",
    [](PointersGen* component, ::Smp::IRequest *request) {
        ::Xsmp::Request::setReturnValue(request, ::Smp::PrimitiveTypeKind::PTK_Int32, component->get_plain());
    }},

    // Setter handler for Property plain
    {"set_plain",
    [](PointersGen* component, ::Smp::IRequest *request) {
        component->set_plain(::Xsmp::Request::get<::Smp::Int32>(component, request, "plain", ::Smp::PrimitiveTypeKind::PTK_Int32));
    }},
};

void PointersGen::Invoke(::Smp::IRequest* request) {
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

const ::Smp::Uuid& PointersGen::GetUuid() const {
    return Uuid_Pointers;
}
} // namespace n
