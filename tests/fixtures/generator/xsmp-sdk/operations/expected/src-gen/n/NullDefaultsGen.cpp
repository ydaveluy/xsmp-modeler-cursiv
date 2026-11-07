// -----------------------------------------------------------------------------
// File Name    : NullDefaultsGen.cpp
// -----------------------------------------------------------------------------
/// @file n/NullDefaultsGen.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <n/NullDefaults.h>
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
NullDefaultsGen::NullDefaultsGen(
    ::Smp::String8 name,
    ::Smp::String8 description,
    ::Smp::IComposite* parent,
    ::Smp::ISimulator* simulator):
    // Base class
    ::Xsmp::Model(name, description, parent, simulator) { }

void NullDefaultsGen::Publish(::Smp::IPublication* receiver) {
    // Call parent class implementation first
    ::Xsmp::Model::Publish(receiver);

    // Publish operation f
    auto* op_f = receiver->PublishOperation(
        "f", // Name
        "", // Description
        ::Smp::ViewKind::VK_All // View Kind
    );
    op_f->PublishParameter(
        "a", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Duration, // Type UUID
        Smp::Publication::ParameterDirectionKind::PDK_Out // Parameter Direction Kind
    );
    op_f->PublishParameter(
        "b", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        Smp::Publication::ParameterDirectionKind::PDK_InOut // Parameter Direction Kind
    );

    // Publish operation g
    auto* op_g = receiver->PublishOperation(
        "g", // Name
        "", // Description
        ::Smp::ViewKind::VK_All // View Kind
    );
    op_g->PublishParameter(
        "c", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        Smp::Publication::ParameterDirectionKind::PDK_In // Parameter Direction Kind
    );

    // Publish operation h
    auto* op_h = receiver->PublishOperation(
        "h", // Name
        "", // Description
        ::Smp::ViewKind::VK_All // View Kind
    );
    op_h->PublishParameter(
        "d", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        Smp::Publication::ParameterDirectionKind::PDK_Out // Parameter Direction Kind
    );
    op_h->PublishParameter(
        "e", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        Smp::Publication::ParameterDirectionKind::PDK_InOut // Parameter Direction Kind
    );

    // Call user DoPublish if any
    ::Xsmp::Component::Helper::Publish<::n::NullDefaults>(this, receiver);
}

void NullDefaultsGen::Configure(::Smp::Services::ILogger* logger, ::Smp::Services::ILinkRegistry* linkRegistry) {
    // Call parent implementation first
    ::Xsmp::Model::Configure(logger, linkRegistry);

    // Call user DoConfigure if any
    ::Xsmp::Component::Helper::Configure<::n::NullDefaults>(this, logger, linkRegistry);
}

void NullDefaultsGen::Connect(::Smp::ISimulator* simulator) {
    // Call parent implementation first
    ::Xsmp::Model::Connect(simulator);

    // Call user DoConnect if any
    ::Xsmp::Component::Helper::Connect<::n::NullDefaults>(this, simulator);
}

void NullDefaultsGen::Disconnect() {
    if (this->::Xsmp::Component::GetState() == ::Smp::ComponentStateKind::CSK_Connected) {
        // Call user DoDisconnect if any
        ::Xsmp::Component::Helper::Disconnect<::n::NullDefaults>(this);
    }

    // Call parent implementation last, to remove references to the Simulator and its services
    ::Xsmp::Model::Disconnect();
}

std::map<std::string_view, std::function<void(NullDefaultsGen*, ::Smp::IRequest*)>> NullDefaultsGen::_requestHandlers{
    // Handler for Operation f
    {"f",
    [](NullDefaultsGen* component, ::Smp::IRequest *request) {
        const bool has_a = request->GetParameterIndex("a") >= 0;
        ::Smp::Duration p_a{};
        const bool has_b = request->GetParameterIndex("b") >= 0;
        ::Smp::Int32 p_b{};
        if (has_b) {
            p_b = ::Xsmp::Request::get<::Smp::Int32>(component, request, "b", ::Smp::PrimitiveTypeKind::PTK_Int32);
        }

        component->f(has_a ? &p_a : nullptr, has_b ? &p_b : nullptr);
        if (has_a) { ::Xsmp::Request::set(component, request, "a", ::Smp::PrimitiveTypeKind::PTK_Duration, p_a); }
        if (has_b) { ::Xsmp::Request::set(component, request, "b", ::Smp::PrimitiveTypeKind::PTK_Int32, p_b); }
    }},

    // Handler for Operation g
    {"g",
    [](NullDefaultsGen* component, ::Smp::IRequest *request) {
        const bool has_c = request->GetParameterIndex("c") >= 0;
        ::Smp::Int32 p_c{};
        if (has_c) {
            p_c = ::Xsmp::Request::get<::Smp::Int32>(component, request, "c", ::Smp::PrimitiveTypeKind::PTK_Int32);
        }

        component->g(has_c ? &p_c : nullptr);
    }},

    // Handler for Operation h
    {"h",
    [](NullDefaultsGen* component, ::Smp::IRequest *request) {
        const bool has_d = request->GetParameterIndex("d") >= 0;
        ::Smp::Int32 p_d{};
        const bool has_e = request->GetParameterIndex("e") >= 0;
        ::Smp::Int32 p_e{};
        if (has_e) {
            p_e = ::Xsmp::Request::get<::Smp::Int32>(component, request, "e", ::Smp::PrimitiveTypeKind::PTK_Int32);
        }

        component->h(has_d ? &p_d : nullptr, has_e ? &p_e : nullptr);
        if (has_d) { ::Xsmp::Request::set(component, request, "d", ::Smp::PrimitiveTypeKind::PTK_Int32, p_d); }
        if (has_e) { ::Xsmp::Request::set(component, request, "e", ::Smp::PrimitiveTypeKind::PTK_Int32, p_e); }
    }},
};

void NullDefaultsGen::Invoke(::Smp::IRequest* request) {
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

const ::Smp::Uuid& NullDefaultsGen::GetUuid() const {
    return Uuid_NullDefaults;
}
} // namespace n
