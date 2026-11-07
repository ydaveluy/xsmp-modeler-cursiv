// -----------------------------------------------------------------------------
// File Name    : DeviceGen.cpp
// -----------------------------------------------------------------------------
/// @file device/DeviceGen.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <device/Device.h>
#include <device/ITarget.h>
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

namespace device
{
//--------------------------- Constructor -------------------------
DeviceGen::DeviceGen(
    ::Smp::String8 name,
    ::Smp::String8 description,
    ::Smp::IComposite* parent,
    ::Smp::ISimulator* simulator):
    // Base class
    ::Xsmp::Model(name, description, parent, simulator),
    // Field state
    state{
        simulator->GetTypeRegistry(), // Type Registry
        ::device::Uuid_State, // Type UUID
        "state", // Name
        "", // Description
        this, // Parent
        ::Smp::ViewKind::VK_All // View Kind
    } { }

void DeviceGen::Publish(::Smp::IPublication* receiver) {
    // Call parent class implementation first
    ::Xsmp::Model::Publish(receiver);

    // Publish field state
    receiver->PublishField(&state);

    // Publish operation ready
    auto* op_ready = receiver->PublishOperation(
        "ready", // Name
        "", // Description
        ::Smp::ViewKind::VK_All // View Kind
    );
    op_ready->PublishParameter(
        "return", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Bool, // Type UUID
        Smp::Publication::ParameterDirectionKind::PDK_Return // Parameter Direction Kind
    );

    // Publish operation setLabel
    auto* op_setLabel = receiver->PublishOperation(
        "setLabel", // Name
        "", // Description
        ::Smp::ViewKind::VK_All // View Kind
    );
    op_setLabel->PublishParameter(
        "value", // Name
        "", // Description
        ::Smp::Uuids::Uuid_String8, // Type UUID
        Smp::Publication::ParameterDirectionKind::PDK_In // Parameter Direction Kind
    );

    // Publish Property enabled
    receiver->PublishProperty(
        "enabled", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Bool, // Type UUID
        ::Smp::AccessKind::AK_ReadWrite, // Access Kind
        ::Smp::ViewKind::VK_All // View Kind
    );

    // Publish Property immutable
    receiver->PublishProperty(
        "immutable", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Bool, // Type UUID
        ::Smp::AccessKind::AK_ReadOnly, // Access Kind
        ::Smp::ViewKind::VK_All // View Kind
    );

    // WARNING: Operation target is not invokable: pointer return values cannot be represented by Smp::AnySimple.

    // Call user DoPublish if any
    ::Xsmp::Component::Helper::Publish<::device::Device>(this, receiver);
}

void DeviceGen::Configure(::Smp::Services::ILogger* logger, ::Smp::Services::ILinkRegistry* linkRegistry) {
    // Call parent implementation first
    ::Xsmp::Model::Configure(logger, linkRegistry);

    // Call user DoConfigure if any
    ::Xsmp::Component::Helper::Configure<::device::Device>(this, logger, linkRegistry);
}

void DeviceGen::Connect(::Smp::ISimulator* simulator) {
    // Call parent implementation first
    ::Xsmp::Model::Connect(simulator);

    // Call user DoConnect if any
    ::Xsmp::Component::Helper::Connect<::device::Device>(this, simulator);
}

void DeviceGen::Disconnect() {
    if (this->::Xsmp::Component::GetState() == ::Smp::ComponentStateKind::CSK_Connected) {
        // Call user DoDisconnect if any
        ::Xsmp::Component::Helper::Disconnect<::device::Device>(this);
    }

    // Call parent implementation last, to remove references to the Simulator and its services
    ::Xsmp::Model::Disconnect();
}

std::map<std::string_view, std::function<void(DeviceGen*, ::Smp::IRequest*)>> DeviceGen::_requestHandlers{
    // Handler for Operation ready
    {"ready",
    [](DeviceGen* component, ::Smp::IRequest *request) {
        ::Xsmp::Request::setReturnValue(request, ::Smp::PrimitiveTypeKind::PTK_Bool, component->ready());
    }},

    // Handler for Operation setLabel
    {"setLabel",
    [](DeviceGen* component, ::Smp::IRequest *request) {
        auto p_value = request->GetParameterValue(request->GetParameterIndex("value"));

        component->setLabel(static_cast<::Smp::String8>(p_value));
    }},

    // Getter handler for Property enabled
    {"get_enabled",
    [](DeviceGen* component, ::Smp::IRequest *request) {
        ::Xsmp::Request::setReturnValue(request, ::Smp::PrimitiveTypeKind::PTK_Bool, component->get_enabled());
    }},

    // Setter handler for Property enabled
    {"set_enabled",
    [](DeviceGen* component, ::Smp::IRequest *request) {
        component->set_enabled(::Xsmp::Request::get<::Smp::Bool>(component, request, "enabled", ::Smp::PrimitiveTypeKind::PTK_Bool));
    }},

    // Getter handler for Property immutable
    {"get_immutable",
    [](DeviceGen* component, ::Smp::IRequest *request) {
        ::Xsmp::Request::setReturnValue(request, ::Smp::PrimitiveTypeKind::PTK_Bool, component->get_immutable());
    }},
};

void DeviceGen::Invoke(::Smp::IRequest* request) {
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

const ::Smp::Uuid& DeviceGen::GetUuid() const {
    return Uuid_Device;
}
} // namespace device
