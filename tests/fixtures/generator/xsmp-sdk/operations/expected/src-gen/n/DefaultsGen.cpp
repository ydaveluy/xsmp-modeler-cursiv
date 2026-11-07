// -----------------------------------------------------------------------------
// File Name    : DefaultsGen.cpp
// -----------------------------------------------------------------------------
/// @file n/DefaultsGen.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <n/Defaults.h>
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
DefaultsGen::DefaultsGen(
    ::Smp::String8 name,
    ::Smp::String8 description,
    ::Smp::IComposite* parent,
    ::Smp::ISimulator* simulator):
    // Base class
    ::Xsmp::Model(name, description, parent, simulator) { }

void DefaultsGen::Publish(::Smp::IPublication* receiver) {
    // Call parent class implementation first
    ::Xsmp::Model::Publish(receiver);

    // Publish operation lost
    auto* op_lost = receiver->PublishOperation(
        "lost", // Name
        "", // Description
        ::Smp::ViewKind::VK_All // View Kind
    );
    op_lost->PublishParameter(
        "a", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        Smp::Publication::ParameterDirectionKind::PDK_In // Parameter Direction Kind
    );
    op_lost->PublishParameter(
        "b", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        Smp::Publication::ParameterDirectionKind::PDK_Out // Parameter Direction Kind
    );

    // Publish operation kept
    auto* op_kept = receiver->PublishOperation(
        "kept", // Name
        "", // Description
        ::Smp::ViewKind::VK_All // View Kind
    );
    op_kept->PublishParameter(
        "a", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        Smp::Publication::ParameterDirectionKind::PDK_Out // Parameter Direction Kind
    );
    op_kept->PublishParameter(
        "b", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        Smp::Publication::ParameterDirectionKind::PDK_In // Parameter Direction Kind
    );

    // Publish operation pointer
    auto* op_pointer = receiver->PublishOperation(
        "pointer", // Name
        "", // Description
        ::Smp::ViewKind::VK_All // View Kind
    );
    op_pointer->PublishParameter(
        "a", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        Smp::Publication::ParameterDirectionKind::PDK_In // Parameter Direction Kind
    );

    // Call user DoPublish if any
    ::Xsmp::Component::Helper::Publish<::n::Defaults>(this, receiver);
}

void DefaultsGen::Configure(::Smp::Services::ILogger* logger, ::Smp::Services::ILinkRegistry* linkRegistry) {
    // Call parent implementation first
    ::Xsmp::Model::Configure(logger, linkRegistry);

    // Call user DoConfigure if any
    ::Xsmp::Component::Helper::Configure<::n::Defaults>(this, logger, linkRegistry);
}

void DefaultsGen::Connect(::Smp::ISimulator* simulator) {
    // Call parent implementation first
    ::Xsmp::Model::Connect(simulator);

    // Call user DoConnect if any
    ::Xsmp::Component::Helper::Connect<::n::Defaults>(this, simulator);
}

void DefaultsGen::Disconnect() {
    if (this->::Xsmp::Component::GetState() == ::Smp::ComponentStateKind::CSK_Connected) {
        // Call user DoDisconnect if any
        ::Xsmp::Component::Helper::Disconnect<::n::Defaults>(this);
    }

    // Call parent implementation last, to remove references to the Simulator and its services
    ::Xsmp::Model::Disconnect();
}

std::map<std::string_view, std::function<void(DefaultsGen*, ::Smp::IRequest*)>> DefaultsGen::_requestHandlers{
    // Handler for Operation lost
    {"lost",
    [](DefaultsGen* component, ::Smp::IRequest *request) {
        auto p_a = ::Xsmp::Request::get<::Smp::Int32>(component, request, "a", ::Smp::PrimitiveTypeKind::PTK_Int32, 1);
        const bool has_b = request->GetParameterIndex("b") >= 0;
        ::Smp::Int32 p_b{};

        component->lost(p_a, has_b ? &p_b : nullptr);
        if (has_b) { ::Xsmp::Request::set(component, request, "b", ::Smp::PrimitiveTypeKind::PTK_Int32, p_b); }
    }},

    // Handler for Operation kept
    {"kept",
    [](DefaultsGen* component, ::Smp::IRequest *request) {
        ::Smp::Int32 p_a{};
        auto p_b = ::Xsmp::Request::get<::Smp::Int32>(component, request, "b", ::Smp::PrimitiveTypeKind::PTK_Int32, 3);

        component->kept(&p_a, p_b);
        ::Xsmp::Request::set(component, request, "a", ::Smp::PrimitiveTypeKind::PTK_Int32, p_a);
    }},

    // Handler for Operation pointer
    {"pointer",
    [](DefaultsGen* component, ::Smp::IRequest *request) {
        const bool has_a = request->GetParameterIndex("a") >= 0;
        ::Smp::Int32 p_a{};
        if (has_a) {
            p_a = ::Xsmp::Request::get<::Smp::Int32>(component, request, "a", ::Smp::PrimitiveTypeKind::PTK_Int32);
        }

        component->pointer(has_a ? &p_a : nullptr);
    }},
};

void DefaultsGen::Invoke(::Smp::IRequest* request) {
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

const ::Smp::Uuid& DefaultsGen::GetUuid() const {
    return Uuid_Defaults;
}
} // namespace n
