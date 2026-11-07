// -----------------------------------------------------------------------------
// File Name    : ProbeGen.cpp
// -----------------------------------------------------------------------------
/// @file optional/ProbeGen.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <optional/Probe.h>
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

namespace optional
{
//--------------------------- Constructor -------------------------
ProbeGen::ProbeGen(
    ::Smp::String8 name,
    ::Smp::String8 description,
    ::Smp::IComposite* parent,
    ::Smp::ISimulator* simulator):
    // Base class
    ::Xsmp::Model(name, description, parent, simulator) { }

void ProbeGen::Publish(::Smp::IPublication* receiver) {
    // Call parent class implementation first
    ::Xsmp::Model::Publish(receiver);

    // Publish operation ope_input
    auto* op_ope_input = receiver->PublishOperation(
        "ope_input", // Name
        "Optional input.", // Description
        ::Smp::ViewKind::VK_All // View Kind
    );
    op_ope_input->PublishParameter(
        "value", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        Smp::Publication::ParameterDirectionKind::PDK_In // Parameter Direction Kind
    );

    // Publish operation ope_string
    auto* op_ope_string = receiver->PublishOperation(
        "ope_string", // Name
        "The request owns the String8 until the handler returns.", // Description
        ::Smp::ViewKind::VK_All // View Kind
    );
    op_ope_string->PublishParameter(
        "value", // Name
        "", // Description
        ::Smp::Uuids::Uuid_String8, // Type UUID
        Smp::Publication::ParameterDirectionKind::PDK_In // Parameter Direction Kind
    );

    // Publish operation ope_output
    auto* op_ope_output = receiver->PublishOperation(
        "ope_output", // Name
        "Optional output.", // Description
        ::Smp::ViewKind::VK_All // View Kind
    );
    op_ope_output->PublishParameter(
        "value", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        Smp::Publication::ParameterDirectionKind::PDK_Out // Parameter Direction Kind
    );

    // Publish operation ope_inout
    auto* op_ope_inout = receiver->PublishOperation(
        "ope_inout", // Name
        "Optional input/output.", // Description
        ::Smp::ViewKind::VK_All // View Kind
    );
    op_ope_inout->PublishParameter(
        "value", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        Smp::Publication::ParameterDirectionKind::PDK_InOut // Parameter Direction Kind
    );

    // Publish operation ope_names
    auto* op_ope_names = receiver->PublishOperation(
        "ope_names", // Name
        "The parameter names must not collide with generated presence flags.", // Description
        ::Smp::ViewKind::VK_All // View Kind
    );
    op_ope_names->PublishParameter(
        "value", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        Smp::Publication::ParameterDirectionKind::PDK_In // Parameter Direction Kind
    );
    op_ope_names->PublishParameter(
        "value_present", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        Smp::Publication::ParameterDirectionKind::PDK_In // Parameter Direction Kind
    );

    // Call user DoPublish if any
    ::Xsmp::Component::Helper::Publish<::optional::Probe>(this, receiver);
}

void ProbeGen::Configure(::Smp::Services::ILogger* logger, ::Smp::Services::ILinkRegistry* linkRegistry) {
    // Call parent implementation first
    ::Xsmp::Model::Configure(logger, linkRegistry);

    // Call user DoConfigure if any
    ::Xsmp::Component::Helper::Configure<::optional::Probe>(this, logger, linkRegistry);
}

void ProbeGen::Connect(::Smp::ISimulator* simulator) {
    // Call parent implementation first
    ::Xsmp::Model::Connect(simulator);

    // Call user DoConnect if any
    ::Xsmp::Component::Helper::Connect<::optional::Probe>(this, simulator);
}

void ProbeGen::Disconnect() {
    if (this->::Xsmp::Component::GetState() == ::Smp::ComponentStateKind::CSK_Connected) {
        // Call user DoDisconnect if any
        ::Xsmp::Component::Helper::Disconnect<::optional::Probe>(this);
    }

    // Call parent implementation last, to remove references to the Simulator and its services
    ::Xsmp::Model::Disconnect();
}

std::map<std::string_view, std::function<void(ProbeGen*, ::Smp::IRequest*)>> ProbeGen::_requestHandlers{
    // Handler for Operation ope_input
    {"ope_input",
    [](ProbeGen* component, ::Smp::IRequest *request) {
        const bool has_value = request->GetParameterIndex("value") >= 0;
        ::Smp::Int32 p_value{};
        if (has_value) {
            p_value = ::Xsmp::Request::get<::Smp::Int32>(component, request, "value", ::Smp::PrimitiveTypeKind::PTK_Int32);
        }

        component->ope_input(has_value ? &p_value : nullptr);
    }},

    // Handler for Operation ope_string
    {"ope_string",
    [](ProbeGen* component, ::Smp::IRequest *request) {
        const bool has_value = request->GetParameterIndex("value") >= 0;
        ::Smp::String8 p_value{};
        ::Smp::AnySimple storage_value;
        if (has_value) {
            storage_value = ::Xsmp::Request::get<::Smp::AnySimple>(component, request, "value", ::Smp::PrimitiveTypeKind::PTK_String8);
            p_value = static_cast<::Smp::String8>(storage_value);
        }

        component->ope_string(has_value ? &p_value : nullptr);
    }},

    // Handler for Operation ope_output
    {"ope_output",
    [](ProbeGen* component, ::Smp::IRequest *request) {
        const bool has_value = request->GetParameterIndex("value") >= 0;
        ::Smp::Int32 p_value{};

        component->ope_output(has_value ? &p_value : nullptr);
        if (has_value) { ::Xsmp::Request::set(component, request, "value", ::Smp::PrimitiveTypeKind::PTK_Int32, p_value); }
    }},

    // Handler for Operation ope_inout
    {"ope_inout",
    [](ProbeGen* component, ::Smp::IRequest *request) {
        const bool has_value = request->GetParameterIndex("value") >= 0;
        ::Smp::Int32 p_value{};
        if (has_value) {
            p_value = ::Xsmp::Request::get<::Smp::Int32>(component, request, "value", ::Smp::PrimitiveTypeKind::PTK_Int32);
        }

        component->ope_inout(has_value ? &p_value : nullptr);
        if (has_value) { ::Xsmp::Request::set(component, request, "value", ::Smp::PrimitiveTypeKind::PTK_Int32, p_value); }
    }},

    // Handler for Operation ope_names
    {"ope_names",
    [](ProbeGen* component, ::Smp::IRequest *request) {
        const bool has_value = request->GetParameterIndex("value") >= 0;
        ::Smp::Int32 p_value{};
        if (has_value) {
            p_value = ::Xsmp::Request::get<::Smp::Int32>(component, request, "value", ::Smp::PrimitiveTypeKind::PTK_Int32);
        }
        const bool has_value_present = request->GetParameterIndex("value_present") >= 0;
        ::Smp::Int32 p_value_present{};
        if (has_value_present) {
            p_value_present = ::Xsmp::Request::get<::Smp::Int32>(component, request, "value_present", ::Smp::PrimitiveTypeKind::PTK_Int32);
        }

        component->ope_names(has_value ? &p_value : nullptr, has_value_present ? &p_value_present : nullptr);
    }},
};

void ProbeGen::Invoke(::Smp::IRequest* request) {
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

const ::Smp::Uuid& ProbeGen::GetUuid() const {
    return Uuid_Probe;
}
} // namespace optional
